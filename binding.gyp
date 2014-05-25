{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "src/addon.cc", "src/cec_wrap.cc", "src/adapter_wrap.cc" ],
      "include_dirs": [
        "../libcec-build/include/libcec"
      ],
      "libraries": [
        "-lcec",
        "-L../libcec-build/lib"
      ]
    }
  ]
}
