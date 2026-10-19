#!/usr/bin/env python3
"""Turns the compiled app into VidGrab.ipa with Python and yt-dlp inside.

Steps: copy the app, add Python.framework and a trimmed standard library,
convert compiled Python modules (.so) into signed frameworks the way iOS
requires, add the app code and packages, precompile, sign, and zip.
"""
import os
import plistlib
import shutil
import subprocess
import sys
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
WORKSPACE = os.environ.get('GITHUB_WORKSPACE', HERE)
XC = os.environ.get('PYTHON_XCFRAMEWORK', '/root/pyios/Python.xcframework')
SLICE = os.path.join(XC, 'ios-arm64')
PY314 = subprocess.check_output(['uv', 'python', 'find', '3.14'], text=True).strip()
LDID = os.environ.get('LDID', '/root/theos/toolchain/linux/iphone/bin/ldid')
YTDLP_ZIP = os.environ.get('YTDLP_ZIP', os.path.join(WORKSPACE, 'Resources', 'yt-dlp.zip'))
PKGS = os.environ.get('APP_PACKAGES', os.path.join(WORKSPACE, 'app_packages'))
BUNDLE_ID = 'com.t4mag0.vidgrab'

STAGE = os.path.join(HERE, 'stage')
APP = os.path.join(STAGE, 'Payload', 'VidGrab.app')
OUT = os.path.join(HERE, 'VidGrab.ipa')

DROP_STDLIB = {'test', 'idlelib', 'tkinter', 'turtledemo', 'ensurepip', 'pydoc_data', '__phello__',
               'turtle.py', 'lib2to3', 'venv'}
DROP_DYNLOAD_PREFIX = ('_test', '_ctypes_test', 'xx', '_xxtestfuzz', '_remote_debugging')

# Mach-O magic numbers (big-endian and little-endian, 32/64-bit, and FAT).
# Any .so that doesn't start with one of these is not an iOS binary (e.g. a
# Linux x86_64 wheel pulled in by pip) and must be skipped — ldid can't sign it.
MACHO_MAGICS = (
    b'\xfe\xed\xfa\xce',  # MH_MAGIC (big-endian 32-bit)
    b'\xce\xfa\xed\xfe',  # MH_CIGAM (little-endian 32-bit)
    b'\xfe\xed\xfa\xcf',  # MH_MAGIC_64 (big-endian 64-bit)
    b'\xcf\xfa\xed\xfe',  # MH_CIGAM_64 (little-endian 64-bit) — iOS arm64
    b'\xca\xfe\xba\xbe',  # FAT_MAGIC (big-endian)
    b'\xbe\xba\xfe\xca',  # FAT_CIGAM (little-endian)
)


def is_macho(path):
    try:
        with open(path, 'rb') as f:
            return f.read(4) in MACHO_MAGICS
    except OSError:
        return False


def run(*cmd):
    subprocess.run(cmd, check=True)


def main():
    print(f'HERE={HERE}')
    print(f'WORKSPACE={WORKSPACE}')
    print(f'YTDLP_ZIP={YTDLP_ZIP} exists={os.path.exists(YTDLP_ZIP)}')
    print(f'PKGS={PKGS} exists={os.path.exists(PKGS)}')
    print(f'XC={XC} exists={os.path.exists(XC)}')

    shutil.rmtree(STAGE, ignore_errors=True)
    os.makedirs(os.path.dirname(APP))
    shutil.copytree(os.path.join(HERE, '.theos/obj/VidGrab.app'), APP)

    # Python.framework (no headers needed at runtime)
    fw = os.path.join(APP, 'Frameworks')
    os.makedirs(fw)
    shutil.copytree(os.path.join(SLICE, 'Python.framework'), os.path.join(fw, 'Python.framework'),
                    ignore=shutil.ignore_patterns('Headers'))

    # Standard library: shared part + arm64 part, trimmed
    lib = os.path.join(APP, 'python', 'lib')
    shutil.copytree(os.path.join(XC, 'lib'), lib,
                    ignore=lambda d, names: [n for n in names if n in DROP_STDLIB or n == '__pycache__'
                                             or n.startswith('libpython')])
    shutil.copytree(os.path.join(SLICE, 'lib-arm64'), lib, dirs_exist_ok=True,
                    ignore=lambda d, names: [n for n in names if n.startswith(DROP_DYNLOAD_PREFIX)
                                             or n.startswith('libpython')])
    for root, dirs, files in os.walk(lib):
        for d in list(dirs):
            if d in ('tests', 'idle_test'):
                shutil.rmtree(os.path.join(root, d))
                dirs.remove(d)

    # App code and packages
    shutil.copytree(os.path.join(HERE, 'app'), os.path.join(APP, 'app'),
                    ignore=shutil.ignore_patterns('__pycache__'))
    pk = os.path.join(APP, 'app_packages')
    shutil.copytree(PKGS, pk, ignore=shutil.ignore_patterns('__pycache__'))
    with zipfile.ZipFile(YTDLP_ZIP) as z:
        for n in z.namelist():
            if n.startswith(('yt_dlp/', 'yt_dlp_ejs/')):
                z.extract(n, pk)

    # Precompile so the app doesn't compile Python on every launch (the bundle is read-only)
    for d in (lib, os.path.join(APP, 'app'), pk):
        run(PY314, '-m', 'compileall', '-q', '-j0', '--invalidation-mode', 'unchecked-hash', d)

    # Convert every .so into Frameworks/<module>.framework as iOS requires.
    # Skip any .so that isn't a Mach-O (e.g. Linux wheels installed by pip).
    with open(os.path.join(XC, 'build', 'iOS-dylib-Info-template.plist'), 'rb') as f:
        template = plistlib.load(f)
    count = 0
    skipped = 0
    for base in (os.path.join(lib, 'python3.14', 'lib-dynload'), pk):
        for root, _, files in os.walk(base):
            for name in files:
                if not name.endswith('.so'):
                    continue
                full = os.path.join(root, name)
                if not is_macho(full):
                    print(f'  skipping non-Mach-O: {os.path.relpath(full, base)}')
                    os.remove(full)
                    skipped += 1
                    continue
                rel = os.path.relpath(full, base)
                module = rel.split('.')[0].replace(os.sep, '.')
                fdir = os.path.join(fw, f'{module}.framework')
                os.makedirs(fdir, exist_ok=True)
                info = dict(template, CFBundleExecutable=module,
                            CFBundleIdentifier=f'{BUNDLE_ID}.{module}'.replace('_', '-'))
                with open(os.path.join(fdir, 'Info.plist'), 'wb') as f:
                    plistlib.dump(info, f)
                shutil.move(full, os.path.join(fdir, module))
                rel_to_app = os.path.relpath(full, APP)
                with open(full[:-3] + '.fwork', 'w') as f:
                    f.write(f'Frameworks/{module}.framework/{module}')
                with open(os.path.join(fdir, f'{module}.origin'), 'w') as f:
                    f.write(rel_to_app[:-3] + '.fwork')
                count += 1
    print(f'Converted {count} Python extension modules to frameworks, skipped {skipped} non-iOS files')

    # Sign (ad-hoc). Sideloading tools re-sign with your own certificate.
    for name in sorted(os.listdir(fw)):
        binary = os.path.join(fw, name, name[:-len('.framework')])
        run(LDID, '-S', binary)
    # Floating download bubble over other apps (TrollStore and jailbreak). Built from hud/.
    hud_src = os.path.join(HERE, 'hud')
    run('make', '-C', hud_src, 'FINALPACKAGE=1')
    hud = os.path.join(APP, 'VidGrabHUD')
    shutil.copy2(os.path.join(hud_src, '.theos', 'obj', 'VidGrabHUD'), hud)
    os.chmod(hud, 0o755)
    run(LDID, '-S' + os.path.join(hud_src, 'entitlements.plist'), hud)
    # Share > VidGrab (share sheet extension). Built from share/.
    share_src = os.path.join(HERE, 'share')
    run('make', '-C', share_src, 'FINALPACKAGE=1')
    plug = os.path.join(APP, 'PlugIns')
    os.makedirs(plug, exist_ok=True)
    appex = os.path.join(plug, 'VGShare.appex')
    shutil.copytree(os.path.join(share_src, '.theos', 'obj', 'VGShare.appex'), appex)
    run(LDID, '-S', os.path.join(appex, 'VGShare'))
    # The app keeps no special permissions: on TrollStore, unsandboxing it turns the browser black,
    # and iOS won't let a sandboxed app start the bubble helper. The bubble over other apps is
    # for jailbreak installs, where the jailbreak starts it as a background service.
    run(LDID, '-S', os.path.join(APP, 'VidGrab'))

    # Everything must be readable by the phone's normal user.
    for r, ds, fs in os.walk(APP):
        for n in ds + fs:
            p = os.path.join(r, n)
            if not os.path.islink(p):
                os.chmod(p, os.stat(p).st_mode | 0o444 | (0o111 if os.path.isdir(p) else 0))

    # Zip
    if os.path.exists(OUT):
        os.remove(OUT)
    with zipfile.ZipFile(OUT, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for root, _, files in os.walk(os.path.join(STAGE, 'Payload')):
            for name in files:
                full = os.path.join(root, name)
                zi = zipfile.ZipInfo.from_file(full, os.path.relpath(full, STAGE))
                zi.compress_type = zipfile.ZIP_DEFLATED
                with open(full, 'rb') as f:
                    z.writestr(zi, f.read(), compresslevel=9)
    print('IPA:', OUT, f'{os.path.getsize(OUT) / 1e6:.1f} MB')


if __name__ == '__main__':
    sys.exit(main())
