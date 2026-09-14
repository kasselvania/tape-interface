"""Finished-artifact routing-probe boundaries, alongside the SDK export gate."""
import subprocess

common = {
    'engine_get_ctx', 'engine_set_callbacks', 'engine_set_active', 'engine_clear_callbacks',
    'mixer_get_in', 'mixer_get_fs', 'os_app_get_model',
    'os_audio_get_monitor', 'os_audio_get_input', 'gfx_set_color',
    'gfx_draw_str', 'gfx_draw_strf', 'ui_hints_set_labels',
}
extra = {
    'route_probe': {'mixer_get_out', 'os_app_exit', 'os_controls_encoder_get_delta',
                    'ui_statusbar_show', 'ui_hints_show', 'sinf', 'gfx_set_font',
                    'gfx_nunito_bold_18', 'gfx_nunito_semibold_14'},
    'route_source_probe': set(),
}
for name, additions in extra.items():
    path = f'build/{name}.tapp'
    output = subprocess.check_output(
        ['llvm-nm', '--undefined-only', '--format=posix', path], text=True)
    imports = {line.split()[0] for line in output.splitlines() if line.strip()}
    allowed = common | additions
    assert imports == allowed, f'{name}: unexpected={imports-allowed}, missing={allowed-imports}'
    print(f'PASS {name}: {len(imports)} public imports, exact probe boundary')
    print('\n'.join(sorted(imports)))
