
commit=`git describe --dirty --tags --always --long`
echo "#define GIT \"${commit}\""

