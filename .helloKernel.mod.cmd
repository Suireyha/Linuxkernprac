savedcmd_helloKernel.mod := printf '%s\n'   helloKernel.o | awk '!x[$$0]++ { print("./"$$0) }' > helloKernel.mod
