set CUR_DIR=%cd%

rem ��ӳ������Ա��뻷��·��
set PATH_JAVA=%JAVA_HOME%\bin;
set PATH_MINGW=D:\code\online-judge\judger-kernel\build\3part\mingw32\bin

set Path=%PATH_JAVA%;
set Path=%Path%%PATH_MINGW%;

SET BIN_NAME=judger.exe

echo ����%BIN_NAME%
%BIN_NAME%