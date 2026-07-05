{ pkgs ? import <nixpkgs> {} }:

let
    picoSdk = pkgs.fetchFromGitHub {
        owner = "raspberrypi";
        repo = "pico-sdk";
        rev = "a1438dff1d38bd9c65dbd693f0e5db4b9ae91779";
        fetchSubmodules = true;
        sha256 = "sha256-8ubZW6yQnUTYxQqYI6hi7s3kFVQhe5EaxVvHmo93vgk="; # this will fail first time
    };
in

pkgs.mkShell {
    name = "lora-packet-transmission";

    buildInputs = with pkgs; [
        clang
        clang-tools
        llvmPackages_latest.lldb 
        llvmPackages_latest.libllvm
        gcc-arm-embedded
        newlib
        cmake
        ninja
        openocd
        picotool
        minicom
    ];

    shellHook = ''
        # set CLANGD_PATH for editor tooling to use the Nix-wrapped clangd
        export CLANGD_PATH="${pkgs.clang-tools}/bin/clangd"

        if [[ -x "$CLANGD_PATH" ]]; then
            echo "✅ CLANGD found at: $CLANGD_PATH"
        else
            echo "❌ ERROR: CLANGD not found at: $CLANGD_PATH"
        fi

        export PICO_SDK_PATH=${picoSdk}
        echo "✅ Pico SDK ready at $PICO_SDK_PATH"

        CLANGD_CONFIG_FILE=".clangd"

        cat > $CLANGD_CONFIG_FILE <<EOF
CompileFlags:
  CompilationDatabase: .
  Remove:
    - "-ffunction-sections"
    - "-fdata-sections"
    - "-m32"
    - "-m64"
    - "-march=*"
    - "-mtune=*"
    - "${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-gcc"
  Add:
    - "-target"
    - "arm-none-eabi"
    - "-mcpu=cortex-m0plus"
    - "-mthumb"
    - "-mfloat-abi=soft"
    - "-nostdinc"
    - "-nostdinc++"
    - "--sysroot=${pkgs.gcc-arm-embedded}/arm-none-eabi"
    - "-isystem"
    - "${pkgs.gcc-arm-embedded}/arm-none-eabi/include"
    - "-isystem"
    - "${pkgs.gcc-arm-embedded}/lib/gcc/arm-none-eabi/14.3.1/include"
    - "-isystem"
    - "${pkgs.gcc-arm-embedded}/lib/gcc/arm-none-eabi/14.3.1/include-fixed"
EOF

        echo "✅ Generated .clangd config for Pico + ARM cross-toolchain"
    '';
}
