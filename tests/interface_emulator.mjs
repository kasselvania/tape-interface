// Drive the unmodified official WASM emulator through the same exports used by
// its web worker. No mocked firmware APIs, audio generation or browser control.
// Download its two files into build/emulator first; see docs/STAGE_C1.md.
import assert from 'node:assert/strict';
import { readFile, writeFile } from 'node:fs/promises';
import { createHash } from 'node:crypto';
import { resolve } from 'node:path';
import { pathToFileURL } from 'node:url';

const directory = resolve('build/emulator');
const moduleBytes = await readFile(`${directory}/tapp_emu.mjs`);
const wasmBytes = await readFile(`${directory}/tapp_emu.wasm`);
const tappBytes = await readFile('build/interface.tapp');
const sha256 = bytes => createHash('sha256').update(bytes).digest('hex');
const logs = [];
const factory = (await import(pathToFileURL(`${directory}/tapp_emu.mjs`))).default;
const emu = await factory({
    wasmBinary: wasmBytes,
    print: text => logs.push(text),
    printErr: text => logs.push(text),
});
const status = () => emu.UTF8ToString(emu._emu_web_status());
async function frames(count) {
    for (let i = 0; i < count; i++) {
        emu._emu_web_render(1);
        await new Promise(resolve => setTimeout(resolve, 40));
    }
}
async function load() {
    const ptr = emu._malloc(tappBytes.length);
    emu.HEAPU8.set(tappBytes, ptr);
    assert.equal(emu._emu_web_load(ptr, tappBytes.length), 1);
    emu._free(ptr);
    await frames(3);
    assert.equal(status(), 'loaded "Tape Interface"');
}
async function gesture(button, hold = false) {
    emu._emu_web_key(button, 1);
    await frames(hold ? 35 : 3);
    emu._emu_web_key(button, 0);
    await frames(3);
}
async function capture(name) {
    const ptr = emu._emu_web_render(1);
    assert.notEqual(ptr, 0);
    const rgba = emu.HEAPU8.slice(ptr, ptr + 400 * 240 * 4);
    const rgb = Buffer.alloc(400 * 240 * 3);
    for (let i = 0, j = 0; i < rgba.length; i += 4, j += 3) {
        rgb[j] = rgba[i]; rgb[j + 1] = rgba[i + 1]; rgb[j + 2] = rgba[i + 2];
    }
    await writeFile(`${directory}/${name}.ppm`,
        Buffer.concat([Buffer.from('P6\n400 240\n255\n'), rgb]));
    return rgb;
}
// Compare only status text, excluding the firmware's animated hint band.
const row = (rgb, y) => rgb.subarray((y - 18) * 400 * 3, (y + 2) * 400 * 3);
await load();
const initial = await capture('initial');
await gesture(0);
const off = await capture('monitor-off');
assert.notDeepEqual(row(initial, 158), row(off, 158));
await gesture(0);
const on = await capture('monitor-on');
assert.deepEqual(row(initial, 158), row(on, 158));
// The official emulator classifies a long gesture as HOLD instead of PRESS.
// BTN1 HOLD must leave the monitor unchanged.
await gesture(0, true);
const held = await capture('monitor-held');
assert.deepEqual(row(on, 158), row(held, 158));
await gesture(1);
const sourcePressed = await capture('source-pressed');
assert.deepEqual(row(initial, 110), row(sourcePressed, 110));
await gesture(1, true);
const mic = await capture('mic');
assert.notDeepEqual(row(initial, 110), row(mic, 110));
await gesture(1, true);
const line = await capture('line');
assert.deepEqual(row(initial, 110), row(line, 110));
emu._emu_web_encoder(5);
await frames(3);
const encoder = await capture('encoder');
assert.equal(status(), 'loaded "Tape Interface"');
// This emulator build lacks mixer_get; turning the wheel must safely keep N/A.
assert.deepEqual(row(initial, 134), row(encoder, 134));
await gesture(3);
assert.equal(status(), 'loaded "Tape Interface"');
await gesture(3, true);
assert.equal(status(), 'tapp exited — drop another');
await load();
await capture('relaunch');

const unresolved = [...new Set(logs.filter(s => s.includes('unresolved API symbol:')))];
assert.deepEqual(unresolved, ['[emu] unresolved API symbol: mixer_get']);
assert(logs.some(s => s.includes('no engine_cb in descriptor')));
assert(logs.some(s => s.includes('tapp requested exit; unloading')));
const evidence = {
    emulator: 'https://tapp.b.edti.me/emu/tapp_emu.js + tapp_emu.wasm',
    sha256: { js: sha256(moduleBytes), wasm: sha256(wasmBytes), tapp: sha256(tappBytes) },
    results: 'PASS load, monitor press/hold, source press/hold/back, null-gain encoder, exit, reload',
    limitation: 'mixer_get unresolved: gain displays N/A; real gain/callback and physical audio NOT TESTED',
    logs: [...new Set(logs)],
};
await writeFile(`${directory}/results.json`, JSON.stringify(evidence, null, 2) + '\n');
console.log(JSON.stringify(evidence, null, 2));
