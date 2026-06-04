const assert = require('assert');
const { promises: fs } = require('fs');

const helpers = require('./helpers');

const safeRequire = (path) => {
  try {
    return require(path);
  } catch (e) {
    if (e.code !== 'MODULE_NOT_FOUND') {
      throw e;
    }
  }
}

const getAddon = () => {
  const addon = safeRequire('../build/Release/readCwd.node') ?? safeRequire('../build/Debug/readCwd.node');

  if (!addon) {
    throw new Error('Failed to require addon');
  }

  return addon;
}

module.exports = (() => {
  switch(process.platform) {
    case 'darwin':
    case 'win32':
      const addon = getAddon();

      return async (processId) => {
        return Promise.resolve(addon.readCwd(processId));
      }
    case 'linux':
      return async (processId) => {
        try {
          const link = await fs.readlink(`/proc/${processId}/cwd`);
          return link.trim();
        } catch (error) {
          return '';
        }
      };
    default:
      throw new Error(`Unsupported platform: ${process.platform}`);
  }
})();
