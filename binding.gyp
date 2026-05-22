{
  "targets": [
    {
      "target_name": "readCwd",
      "dependencies": [
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except"
      ],
      "conditions": [
        [
          "OS=='mac'", {
            "libraries": [
              "-lproc"
            ],
            "sources": [
              "src/darwin/readCwd.cpp"
            ]
          }
        ],
        [
          "OS=='win'", {
            "sources": [
              "src/readCwd.cpp"
            ]
          }
        ]
      ]
    }
  ]
}