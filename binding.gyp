{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "src/addon.cc", "src/cec_wrap.cc", "src/adapter_wrap.cc", "src/types.cc" ],
      "include_dirs": [
        "/usr/local/include/libcec/",
        "/usr/local/include/node/",
        "<!(node -e \"require('nan')\")"
      ],
      "libraries": [
        "-lcec",
        "-L/usr/local/lib/libcec/"
      ]
    }
  ]
}
