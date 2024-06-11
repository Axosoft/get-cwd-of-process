const assert = require('assert');
const { promises: fs } = require('fs');

const helpers = require('./helpers');

const platformFunctions = {
  darwin: async (processId) => {
    try {
      /*
       * -a: and selection of -d and -p
       * -d cwd: only include cwd file descriptor (speedup)
       * -b: avoid potentially blocking calls (speedup, unnecessary for our purposes: getting the cwd)
       * -w: don't print warning messages (like from -b)
       * -P: don't convert port numbers to names (speedup)
       * -l: don't convert user ID numbers to names (speedup)
       * -n: don't convert network numbers to host names for network files (speedup)
       * -Ffn: -F formats output for machines (f: file descriptors, n: file name)
       * -p: PID to lookup
       */
      const { stdout } = await helpers.exec(`lsof -a -d cwd -bwPln -Ffn -p ${processId}`);
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
    if (process.platform !== 'win32') {
      return () => {
        throw new Error('incorrect platform');
      };
    }

    const safeRequire = (path) => {
      try {
        return require(path);
      } catch (e) {
        if (e.code !== 'MODULE_NOT_FOUND') {
          throw e;
        }
      }
    }

    const addon = safeRequire('../build/Release/readCwd.node') ?? safeRequire('../build/Debug/readCwd.node');
    if (!addon) {
      throw new Error('Failed to require addon');
    }

    return async (processId) => {
      return Promise.resolve(addon.readCwd(processId));
    }
  })()
};

module.exports = platformFunctions[process.platform];
