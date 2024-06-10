{
  "targets": [
    {
      "target_name": "readCwd",
      "dependencies": [
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except"
      ],
      "sources": [
        "src/readCwd.cpp",
      ],
    }
  ]
}