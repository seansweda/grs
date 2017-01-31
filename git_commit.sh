
commit=`git describe --abbrev=6 --dirty --always`
echo "#define GIT \"${commit}\""

