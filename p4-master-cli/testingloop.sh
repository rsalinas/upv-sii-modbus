find |entr -cr bash -c 'make -C ../build/Desktop_Qt_6_8_1-Debug/$(basename $PWD) && timeout 3 ../build/Desktop_Qt_6_8_1-Debug/p4-master-cli/p4-master-cli  -1 && git ci -am WIP'
