load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_library(
    name = "core",
    srcs = ["core.cc"],
    hdrs = ["core.h"],
    deps = [
        "@boost.graph",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "core_test",
    srcs = ["core_test.cc"],
    deps = [
        ":core",
        "@googletest//:gtest_main",
    ],
)
