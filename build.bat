@echo off
echo -- Building monitor --
msbuild app-monitor.vcxproj -verbosity:minimal -nologo -property:Configuration=%1 -property:Platform=x64

echo.
echo -- Building test app --
msbuild test-app/test-app.vcxproj -verbosity:minimal -nologo -property:Configuration=Debug -property:Platform=x64

