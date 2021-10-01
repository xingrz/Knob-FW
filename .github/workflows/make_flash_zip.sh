#/bin/bash
pushd build
zip ../flash.zip flash_args
cat flash_args | while read _ f; do
    [[ -f $f ]] && zip ../flash.zip $f
done
popd
