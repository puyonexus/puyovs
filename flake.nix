{
  description = "Multi-player puzzle game similar to Puyo Puyo";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/master";
  };
  outputs = { self, nixpkgs, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forEachSystem = nixpkgs.lib.genAttrs systems;
    in {
      overlays.default = final: prev: {
        puyovs = final.stdenv.mkDerivation {
          pname = "puyovs";
          version = "32";
          src = ./.;
          nativeBuildInputs = with final; with libsForQt5; [
            cmake
            ninja
            qt5.wrapQtAppsHook
            pkg-config
          ];
          buildInputs = with final; with libsForQt5; [
            qt5.qtbase
            zlib-ng
            glm
            jsoncpp
            SDL2
            SDL2_ttf
            libspng
          ];
          cmakeFlags = [
            "-GNinja"
          ];
          meta = {
            mainProgram = "PuyoVS";
          };
        };
      };
      packages = forEachSystem (system:
        let
          pkgs = (import nixpkgs {
            inherit system;
            overlays = [ self.overlays.default ];
          });
        in rec {
          inherit (pkgs) puyovs;
          default = puyovs;
        });
    };
}

