@echo off
rem ==============================================================================
rem Script điều khiển STM32F103 trên Windows: Build, Flash, Erase, GUI Dashboard
rem Dự án: Project_cuong (STM32F103C8T6 StdPeriph + FreeRTOS)
rem ==============================================================================

chcp 65001 >nul
setlocal enabledelayedexpansion

set PROJECT_DIR=%~dp0
cd /d "%PROJECT_DIR%"

rem Kiểm tra lệnh make (hỗ trợ cả make và mingw32-make)
set MAKE_CMD=make
where make >nul 2>&1
if errorlevel 1 (
    where mingw32-make >nul 2>&1
    if not errorlevel 1 (
        set MAKE_CMD=mingw32-make
    )
)

rem Xử lý tham số dòng lệnh CLI nếu có truyền vào
if "%1"=="build" goto do_build
if "%1"=="rebuild" goto do_rebuild
if "%1"=="flash" goto do_flash
if "%1"=="clean" goto do_clean
if "%1"=="erase" goto do_erase
if "%1"=="gui" goto do_gui
if "%1"=="dashboard" goto do_gui
if "%1"=="all" goto do_all
if "%1"=="help" goto show_help
if "%1"=="/?" goto show_help
if "%1"=="-h" goto show_help

:menu
cls
echo ========================================================
echo      STM32F103C8T6 TOOL CONTROLLER - WINDOWS
echo ========================================================
echo   1) Build             - Biên dịch firmware (%MAKE_CMD%)
echo   2) Rebuild           - Dọn dẹp và biên dịch lại sạch sẽ
echo   3) Flash             - Nạp firmware xuống chip (ST-Link / OpenOCD)
echo   4) Build + Flash     - Biên dịch và nạp luôn
echo   5) Erase Chip        - Xóa toàn bộ bộ nhớ Flash của chip
echo   6) Mở Dashboard GUI  - Giao diện đồ họa Python (Serial JSON)
echo   7) Mở Serial Terminal- Xem log UART (python miniterm)
echo   8) Clean             - Xóa thư mục build
echo   0) Thoát
echo ========================================================
set /p choice="Chọn chức năng [0-8]: "

if "%choice%"=="1" goto do_build
if "%choice%"=="2" goto do_rebuild
if "%choice%"=="3" goto do_flash
if "%choice%"=="4" goto do_build_and_flash
if "%choice%"=="5" goto do_erase
if "%choice%"=="6" goto do_gui
if "%choice%"=="7" goto do_miniterm
if "%choice%"=="8" goto do_clean
if "%choice%"=="0" goto exit_script

echo [LỖI] Lựa chọn không hợp lệ!
timeout /t 2 >nul
goto menu

:do_build
echo [1/2] Đang biên dịch firmware...
%MAKE_CMD% -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo [LỖI] Biên dịch thất bại! Vui lòng kiểm tra lại code.
    if "%1"=="" pause
    goto menu
)
echo [OK] Biên dịch thành công! File: build\stm32f103_stdperiph.bin
if "%1"=="" pause
goto menu

:do_rebuild
echo Đang xóa bản cũ và biên dịch lại...
%MAKE_CMD% clean
%MAKE_CMD% -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo [LỖI] Biên dịch thất bại!
    if "%1"=="" pause
    goto menu
)
echo [OK] Biên dịch thành công!
if "%1"=="" pause
goto menu

:do_flash
if not exist "build\stm32f103_stdperiph.elf" (
    echo Chưa thấy file build, tiến hành biên dịch trước...
    %MAKE_CMD% -j%NUMBER_OF_PROCESSORS%
    if errorlevel 1 goto menu
)
echo >>> Đang nạp firmware xuống chip (ST-Link / OpenOCD)...
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program build/stm32f103_stdperiph.elf verify reset exit"
if errorlevel 1 (
    echo [LỖI] Nạp firmware thất bại!
    echo Vui lòng kiểm tra lại cáp kết nối ST-Link với máy tính.
) else (
    echo [OK] NẠP THÀNH CÔNG! Firmware mới đã được ghi vào chip STM32.
)
if "%1"=="" pause
goto menu

:do_build_and_flash
%MAKE_CMD% -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo [LỖI] Biên dịch thất bại!
    pause
    goto menu
)
echo >>> Đang nạp firmware...
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program build/stm32f103_stdperiph.elf verify reset exit"
pause
goto menu

:do_erase
echo >>> ĐANG XÓA TOÀN BỘ FLASH TRÊN STM32F103...
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "init; reset halt; stm32f1x mass_erase 0; reset run; exit"
pause
goto menu

:do_gui
echo Đang khởi chạy giao diện Dashboard Python...
python gui_app.py
if errorlevel 1 (
    echo [CẢNH BÁO] Không thể chạy python gui_app.py!
    echo Hãy đảm bảo đã cài đặt: pip install PyQt5 pyserial
    pause
)
goto menu

:do_miniterm
echo Danh sách cổng COM hiện có:
mode | findstr /C:"COM"
set /p com_port="Nhập tên cổng COM (ví dụ COM3): "
if "%com_port%"=="" set com_port=COM3
python -m serial.tools.miniterm %com_port% 115200
goto menu

:do_clean
%MAKE_CMD% clean
echo [OK] Đã xóa thư mục build!
pause
goto menu

:do_all
%MAKE_CMD% -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo [LỖI] Biên dịch thất bại!
    exit /b 1
)
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program build/stm32f103_stdperiph.elf verify reset exit"
python gui_app.py
exit /b 0

:show_help
echo Cách sử dụng trên Windows:
echo   run.bat                  : Mở Menu tương tác
echo   run.bat build            : Biên dịch dự án
echo   run.bat rebuild          : Dọn dẹp và biên dịch lại
echo   run.bat flash            : Nạp firmware qua ST-Link
echo   run.bat all              : Build + Flash + Mở Dashboard GUI
echo   run.bat gui              : Mở Giao diện Dashboard GUI Python
echo   run.bat erase            : Xóa sạch bộ nhớ Flash chip
echo   run.bat clean            : Xóa thư mục build
exit /b 0

:exit_script
echo Tạm biệt!
exit /b 0
