const assert = require('assert');
const { promises: fs } = require('fs');
const path = require('path');

const helpers = require('./helpers');

const platformFunctions = {
  darwin: async (processId) => {
    try {
      const { stdout } = await helpers.exec(`lsof -Ffn -p ${processId}`);
      const searchString = 'fcwd\nn';
      let startingIndex = stdout.indexOf(searchString);
      if (startingIndex === -1) {
        assert.fail('Case should not occur');
      }

      startingIndex += searchString.length;
      if (startingIndex >= stdout.length) {
        assert.fail('Case should not occur');
      }

      const indexOfNextNewLine = stdout.indexOf('\n', startingIndex);
      if (indexOfNextNewLine === -1) {
        assert.fail('Case should not occur');
      }

      return stdout.substring(startingIndex, indexOfNextNewLine);
    } catch (error) {
      return '';
    }
  },
  linux: async (processId) => {
    try {
      const link = await fs.readlink(`/proc/${processId}/cwd`);
      return link.trim();
    } catch (error) {
      return '';
    }
  },
  win32: (() => {
    const readCwd = helpers.bin('readCwd.exe');
    const readCwd32 = helpers.bin('readCwd32.exe');
    let isPlatform64Bit = null;
    const getIsPlatform64Bit = async () => {
      const { stdout } = await helpers.exec(
        '[Environment]::Is64BitOperatingSystem',
        { shell: 'powershell.exe' }
      );

      return stdout.startsWith('True');
    };

    return async (processId) => {
      if (isPlatform64Bit === null) {
        isPlatform64Bit = await getIsPlatform64Bit();
      }

      try {
        const { stdout } = await helpers.execFile(isPlatform64Bit ? readCwd : readCwd32, [processId]);
        return stdout.trim();
      } catch (error) {
        return '';
      }
    };
  })()
};

module.exports = platformFunctions[process.platform];
