# get-cwd-of-process
This is an npm module for retrieving the CWD of a given process. Works cross-platform. Does not ship prebuilt executables, specifically to be transparent about what this package does. MIT licensed.

## Windows installation
**Requires VS2017 or newer to install**
This package is primarily intended to be _built_ on a 64bit platform, even though it supports CWD detection on both 32bit and 64bit. This is because we build 2 CWD lookup executables, one 32 bit, one 64 bit. Once we know whether you're 64bit or 32bit, we then decide which executable to use for the lifetime of the module.

## Usage
```javascript
const getCwdOfProcess = require('get-cwd-of-process');
async function Foo() {
  const cwd = await getCwdOfProcess(12345);
  console.log(cwd);
}
```
