# get-cwd-of-process
This is an npm module for retrieving the CWD of a given process. Works cross-platform. MIT licensed.

## Usage
```javascript
const getCwdOfProcess = require('get-cwd-of-process');
async function Foo() {
  const cwd = await getCwdOfProcess(12345);
  console.log(cwd);
}
```
