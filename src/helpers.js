const childProcess = require('child_process');
const path = require('path');
const util = require('util');

let _exec;
let _execFile;
module.exports = {
  bin: (name) => path.join(__dirname, '..', 'bin', name),
  get exec() {
    if (!_exec) {
      _exec = util.promisify(childProcess.exec);
    }

    return _exec;
  },
  get execFile() {
    if (!_execFile) {
      _execFile = util.promisify(childProcess.execFile);
    }

    return _execFile;
  }
};
