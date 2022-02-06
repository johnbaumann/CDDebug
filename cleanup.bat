@echo off

cls

REM Regular Build Stuff

del *.rom
REM del *.exe
del *.sym
del *.map
del *.obj
del *.cpe
del *.dep
del *.temp
del *.crunch
del *.o

del comport.txt

del stderr.txt
del stdout.txt

REM IDA stuff

del *.idb
del *.til
del *.id0
del *.id1
del *.nam
del *.i64


call dockermake -f buildme.mk clean



