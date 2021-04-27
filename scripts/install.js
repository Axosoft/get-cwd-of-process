const childProcess = require('child_process');
const { promises: fs } = require('fs');
const path = require('path');
const { promisify } = require('util');
const findVisualStudio = promisify(require('node-gyp/lib/find-visualstudio'));
const exec = promisify(childProcess.exec);
const execFile = promisify(childProcess.execFile);

const die = (msg) => {
  console.log(msg);
  process.exit(1);
};

const getMSBuild = async () => {
  try {
    return (await findVisualStudio(null, '2017')).msBuild;
  } catch (error) {
    return '';
  }
};

// This package is required to be _built_ on a 64 bit platform, but it can be shipped on a 32bit one.
const getIsPlatform64Bit = async () => {
  try {
    const { stdout } = await exec('[Environment]::Is64BitOperatingSystem', { shell: 'powershell.exe' });
    return stdout.startsWith('True');
  } catch (error) {
    die('Failed to detect platform architecture');
  }
};

const moveFile = async (originalFile, newFile) => {
  await fs.mkdir(path.dirname(newFile), { recursive: true });
  await fs.rename(originalFile, newFile);
};

const buildModule = async (msBuild, platform) => {
  await execFile(
    msBuild,
    [`/p:Configuration=Release;Platform=${platform}`],
    { cwd: path.join(__dirname, '..', 'msvs') }
  );
  if (platform === 'x64') {
    await moveFile('msvs\\x64\\Release\\readCwd.exe', 'bin\\readCwd.exe');
    await fs.rmdir('msvs\\x64', { recursive: true });
  } else {
    await moveFile('msvs\\Release\\readCwd.exe', 'bin\\readCwd32.exe');
    await fs.rmdir('msvs\\Release', { recursive: true });
  }
};

const main = async () => {
  if (process.platform !== 'win32') {
    return;
  }

  const msBuild = await getMSBuild();
  if (!msBuild) {
    die('Visual Studio Build Tools 2017 is required to build this package.');
  }

  const isPlatform64Bit = await getIsPlatform64Bit();
  if (!isPlatform64Bit) {
    die('Package must be built on a 64bit architecture, even though it supports 32bit platforms.');
  }

  buildModule(msBuild, 'x86');
  buildModule(msBuild, 'x64');
};

main();