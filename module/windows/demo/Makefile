LIBDIR = ..\src
BUILDDIR = build

LIBS = User32.lib Shell32.lib Version.lib Shlwapi.lib Gdiplus.lib Gdi32.lib Windowsapp.lib

all: create_dir demo

demo: $(BUILDDIR)\demo.exe

$(BUILDDIR)\demo.exe: $(BUILDDIR)\*.obj $(BUILDDIR)\main.obj
	@echo Linking demo.exe
	link /out:$(BUILDDIR)\demo.exe $(BUILDDIR)\*.obj $(LIBS)

$(BUILDDIR)\main.obj: main.cpp
	@echo Compiling main.cpp for demo
	cl /c /EHsc /Fo$(BUILDDIR)\ main.cpp

{$(LIBDIR)}.cpp{$(BUILDDIR)}.obj ::
	@echo Compiling $<
	cl /c /EHsc /Fo$(BUILDDIR)\ $<

create_dir:
	@if not exist $(BUILDDIR) mkdir $(BUILDDIR)

clean:
	@if exist $(BUILDDIR) rmdir /s /q $(BUILDDIR)

run:
	$(BUILDDIR)\demo.exe
