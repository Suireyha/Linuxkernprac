savedcmd_kernsum.mod := printf '%s\n'   kernsum.o | awk '!x[$$0]++ { print("./"$$0) }' > kernsum.mod
