const std = @import("std");

fn concat(a: []const u8, b: []const u8, allocator: std.mem.Allocator) ![]u8 {
    const result = try allocator.alloc(u8, a.len + b.len);

    std.mem.copy(u8, result, a);
    std.mem.copy(u8, result[a.len..], b);

    return result;
}

fn ascending(context: std.mem.Allocator, a: []const u8, b: []const u8) bool {
    _ = context;

    return std.mem.eql(u8, a, b);
}

fn findWindowsKitsAndAddItsLibraryPath(b: *std.build.Builder, lib: *std.Build.CompileStep) !void {
    const windows_kits_lib_path = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib";
    const um_subpath = "\\um\\x64";

    var windows_kits_lib_dir = try std.fs.openIterableDirAbsolute(windows_kits_lib_path, .{
        .access_sub_paths = true,
    });

    var dirs = std.ArrayList([]const u8).init(b.allocator);
    defer dirs.deinit();

    var it = windows_kits_lib_dir.iterate();
    while (try it.next()) |dir| {
        if (dir.kind != .Directory) continue;

        try dirs.append(b.dupe(dir.name));
    }

    if (dirs.getLastOrNull() == null) return error.NoVersionAvailable;

    std.sort.sort([]const u8, dirs.items, b.allocator, ascending);

    const version = dirs.getLast();
    const version_subpath = try concat("\\", version, b.allocator);
    defer b.allocator.free(version_subpath);

    const windows_kits_lib_version_path = try concat(windows_kits_lib_path, version_subpath, b.allocator);
    defer b.allocator.free(windows_kits_lib_version_path);

    const windows_kits_lib_full_path = try concat(windows_kits_lib_version_path, um_subpath, b.allocator);
    defer b.allocator.free(windows_kits_lib_full_path);

    lib.addLibraryPath(windows_kits_lib_full_path);
}

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const directx_guids_lib = b.addStaticLibrary(.{
        .name = "DirectX-Guids",
        // .root_source_file = .{ .path = "<placeholder>" },
        .target = target,
        .optimize = optimize,
    });

    {
        directx_guids_lib.linkLibCpp();

        directx_guids_lib.addIncludePath("include");
        directx_guids_lib.addIncludePath("include/wsl/stubs");

        directx_guids_lib.addCSourceFiles(&.{
            "src/dxguids.cpp",
        }, &.{
            "-Wno-macro-redefined",
            "-Wno-unused-variable",

            "-g",
            "-std=c++14",
        });
    }

    b.installArtifact(directx_guids_lib);

    const directx_headers_lib = b.addStaticLibrary(.{
        .name = "DirectX-Headers",
        // .root_source_file = .{ .path = "<placeholder>" },
        .target = target,
        .optimize = optimize,
    });

    {
        directx_headers_lib.linkLibCpp();

        switch (directx_headers_lib.target.getOsTag()) {
            .windows => {
                try findWindowsKitsAndAddItsLibraryPath(b, directx_headers_lib);

                directx_headers_lib.linkSystemLibraryName("dxcore");
                directx_headers_lib.linkSystemLibraryName("d3d12");
            },
            else => unreachable,
        }

        directx_headers_lib.addIncludePath("include");
        directx_headers_lib.addIncludePath("include/directx");
        directx_headers_lib.addIncludePath("include/wsl/stubs");

        directx_headers_lib.addCSourceFiles(&.{
            "src/d3dx12_property_format_table.cpp",
        }, &.{
            "-Wno-macro-redefined",
            "-Wno-non-virtual-dtor",
            "-Wno-unused-variable",

            "-g",
            "-std=c++14",
        });
    }

    b.installArtifact(directx_headers_lib);

    const directx_headers_test = b.addExecutable(.{
        .name = "DirectX-Headers-Test",
        // .root_source_file = .{ .path = "<placeholder>" },
        .target = target,
        .optimize = optimize,
    });

    {
        directx_headers_test.install();
        directx_headers_test.linkLibCpp();

        directx_headers_test.linkLibrary(directx_guids_lib);
        directx_headers_test.linkLibrary(directx_headers_lib);

        directx_headers_test.addIncludePath("include");

        directx_headers_test.addCSourceFiles(&.{
            "test/test.cpp",
        }, &.{
            "-Wno-switch",

            "-g",
            "-std=c++17",
        });
    }

    const directx_headers_check_feature_support_test = b.addExecutable(.{
        .name = "DirectX-Headers-Check-Feature-Support-Test",
        // .root_source_file = .{ .path = "<placeholder>" },
        .target = target,
        .optimize = optimize,
    });

    {
        directx_headers_check_feature_support_test.install();
        directx_headers_check_feature_support_test.linkLibCpp();

        directx_headers_check_feature_support_test.linkLibrary(directx_guids_lib);
        directx_headers_check_feature_support_test.linkLibrary(directx_headers_lib);

        directx_headers_check_feature_support_test.addIncludePath("include");

        directx_headers_check_feature_support_test.addCSourceFiles(&.{
            "test/feature_check_test.cpp",
        }, &.{
            "-g",
            "-std=c++17",
        });
    }
}
