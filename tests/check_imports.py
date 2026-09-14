"""Check the finished ELF's C1 control-only import boundary, not just its source."""
import subprocess
import sys

allowed = {
    "gfx_draw_str", "gfx_draw_strf", "gfx_set_color", "os_app_exit",
    "os_app_get_model", "ui_hints_set_labels", "ui_hints_show", "ui_statusbar_show",
    "os_audio_get_monitor", "os_audio_set_monitor", "os_audio_get_input",
    "os_audio_switch_input", "os_controls_encoder_get_delta", "mixer_get",
    "mixer_get_input_vol", "param_write_delta_val", "param_update_fast",
    "param_val_percent",
}
output = subprocess.check_output(
    ["llvm-nm", "--undefined-only", "--format=posix", sys.argv[1]], text=True
)
imports = {line.split()[0] for line in output.splitlines() if line.strip()}
unexpected = imports - allowed
if not imports or unexpected:
    raise SystemExit(f"FAIL C1 imports: empty={not imports}, unexpected={sorted(unexpected)}")
print(f"PASS C1 control-only imports ({len(imports)}):")
print("\n".join(sorted(imports)))
