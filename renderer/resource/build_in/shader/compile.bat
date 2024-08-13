cd ./test
for /r %%i in (*.spv) do (
	del "%%i"
)

for /r %%i in (*.frag, *.vert, *.comp, *.geom, *.rchit, *.rmiss, *.rgen, *.rahit, *.rint) do (
	glslangValidator.exe -g --target-env vulkan1.2 -V "%%i" -o "%%i".spv
)
cd ../