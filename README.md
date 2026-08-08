# GSC Obfuscate

Test project to obfuscate GSC files for Call of Duty: Black Ops III.

## Download

- [Latest release](https://github.com/ate47/gsc-obf/releases/latest)
- [Latest build](https://github.com/ate47/gsc-obf/releases/tag/latest_build) (might be unstable)

## Usage

Use the executable on the gsc file or on the fastfile you want to obfuscate.

```pwsh
gsc-obf.exe [files*]
```

The file will be obfuscated and saved into the directory `output`. You can now share this file with less risk that it will be decompiled.

## Current features

Here are the current features supported by the obfuscator.

- [Remove export checksums](docs/features/remove_export_checksums.md)
- [Remove variable names](docs/features/remove_vars.md)
- [Remove private function names](docs/features/remove_privates.md)
- [Remove dev strings information](docs/features/remove_dev_strings_info.md)
- [Remove dev call function names](docs/features/remove_dev_calls_info.md)
- [Add junk in dev blocks](docs/features/add_dev_junk.md)
- [Use trampolines to remove the preamble](docs/features/trampolines.md)
- [Rename fastfile builder](docs/features/ff_rename_builder.md)
- [Change fastfile compression](docs/features/ff_change_compression.md)

## Information

Known Black Ops III GSC decompilers

- [Atian Tools](https://github.com/ate47/atian-cod-tools)
- [Scobalula Cerberus](https://github.com/Scobalula/Cerberus-Repo)
- [shiversoftdev T89-Dec (Cerberus fork)](https://github.com/shiversoftdev/T89-Dec)
- [Xensik gsc-tool](https://github.com/xensik/gsc-tool)
