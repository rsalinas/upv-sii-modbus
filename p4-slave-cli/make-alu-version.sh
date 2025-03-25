for f in *.h *.cpp ; do unifdef -DSTUDENT_VERSION  $f > ${PWD}-alu/$f ; done
