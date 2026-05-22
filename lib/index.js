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

const getReadCwdWithAddon = () => {
  const addon = getAddon();

  return async (processId) => {
    return Promise.resolve(addon.readCwd(processId));
  }
}

const platformFunctions = {
  darwin: getReadCwdWithAddon(),
  win32: getReadCwdWithAddon(),
  linux: async (processId) => {
    try {
      const link = await fs.readlink(`/proc/${processId}/cwd`);
      return link.trim();
    } catch (error) {
      return '';
    }
  },
};

module.exports = platformFunctions[process.platform];
