savedcmd_elev.mod := printf '%s\n'   elev.o | awk '!x[$$0]++ { print("./"$$0) }' > elev.mod
