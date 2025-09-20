{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "imgui-stuff";
  buildInputs = with pkgs; [
    clang_18
    glfw
    glew
  ];
}
