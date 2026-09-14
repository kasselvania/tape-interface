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
let tappBytes;
let expectedName;
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
    assert.equal(status(), `loaded "${expectedName}"`);
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

// UI and lifecycle only. Native tests verify sample-buffer behavior separately.
const row = (rgb, y) => rgb.subarray((y - 18) * 400 * 3, (y + 2) * 400 * 3);
tappBytes = await readFile('build/route_probe.tapp'); expectedName = 'Routing Probe';
await load();
const initial = await capture('route-initial');
await gesture(0); const toneDisarmed = await capture('route-tone-disarmed');
assert.notDeepEqual(row(initial, 102), row(toneDisarmed, 102));
await gesture(1); const pressed = await capture('route-short-arm');
assert.deepEqual(row(toneDisarmed, 198), row(pressed, 198));
await gesture(1, true); const armed = await capture('route-tone-armed');
assert.notDeepEqual(row(pressed, 198), row(armed, 198));
await gesture(2); const stopped = await capture('route-stopped');
assert.deepEqual(row(initial, 102), row(stopped, 102));
assert.deepEqual(row(initial, 198), row(stopped, 198));
for(let i=0;i<4;i++) await gesture(0);
await gesture(1, true); await capture('route-anti-armed');
await gesture(3, true);
assert.match(status(), /tapp exited/);
await load(); const reloaded=await capture('route-reload');
assert.deepEqual(row(initial,198),row(reloaded,198));
await gesture(3,true);
tappBytes = await readFile('build/route_source_probe.tapp'); expectedName = 'Routing Source Probe';
await load(); const srcZero=await capture('source-zero');
await gesture(0); const srcPress=await capture('source-press');
assert.deepEqual(row(srcZero,104),row(srcPress,104));
await gesture(0,true); const srcPass=await capture('source-pass');
assert.notDeepEqual(row(srcZero,104),row(srcPass,104));
await gesture(1); const srcStop=await capture('source-stop');
assert.deepEqual(row(srcZero,104),row(srcStop,104));
assert(!logs.some(s=>s.includes('unresolved API symbol')), logs.join('\n'));
const results={
  runtime: {js:sha256(moduleBytes), wasm:sha256(wasmBytes)},
  artifacts: Object.fromEntries(await Promise.all(['route_probe','route_source_probe'].map(async n=>[n,sha256(await readFile(`build/${n}.tapp`))]))),
  result:'PASS standalone modes, press/hold arming, zero, warning rows, exit/reload; source zero/pass/stop UI',
  limitation:'No physical USB Capture, USB Playback, Speaker, Output Jack or tape routing acceptance. Source background/unload and WET behavior require hardware.',
  logs,
};
await writeFile(`${directory}/routing-results.json`,JSON.stringify(results,null,2));
console.log(JSON.stringify(results,null,2));
