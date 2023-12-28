{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";
    devenv.url = "github:cachix/devenv";
  };

  outputs = {
    self,
    nixpkgs,
    devenv,
    ...
  } @ inputs: let
    pkgs = nixpkgs.legacyPackages."x86_64-linux";
  in {
    devShell.x86_64-linux = devenv.lib.mkShell {
      inherit inputs pkgs;
      modules = [
        ({pkgs, ...}: {
          packages = [pkgs.cmake pkgs.ninja pkgs.gcc pkgs.libgccjit pkgs.linuxPackages_latest.perf];
          enterShell = ''
            export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/external/macrohard/client:$(pwd)/external/macrohard/server
            '';
        })
      ];
    };
  };
}
