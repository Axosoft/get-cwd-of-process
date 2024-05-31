{
  "targets": [
    {
      "target_name": "readCwd",
      "sources": [
        "src/readCwd.cpp",
      ],
      "include_dirs": [
        "<!(node -p \"require('node-addon-api').include_dir\")"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS",
        "NODE_ADDON_API_ENABLE_MAYBE"
      ],
    }
  ]
}