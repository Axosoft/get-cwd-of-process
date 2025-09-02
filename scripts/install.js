const path = require('path');
var spawn = require("child_process").spawn;

const main = async () => {
  if (process.platform !== 'win32') {
    return;
  }

  const nodeGypPath = path.resolve(path.join(__dirname, '..', 'node_modules', '.bin', 'node-gyp.cmd'));
  const args = ['configure', 'rebuild'];

  await new Promise((resolve, reject) => {
    const spawnedProc = spawn(`"${nodeGypPath}" ${args.join(' ')}`, { stdio: 'inherit', shell: true });

    spawnedProc.on('close', (code) => {
      if (code === 0) {
        resolve();
      } else {
        reject(code);
      }
    })
  })
};

main();