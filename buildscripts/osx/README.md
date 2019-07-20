`build-deps.sh`

    Builds shippable versions of Qt, Tor, and OpenSSL. Assumes that a static libevent and pkg-config are available, such as through homebrew.

`build.sh`

    Build Ricochet using those libraries. Set CODESIGN_ID in environment to sign the output; often this is "Developer ID Application".

