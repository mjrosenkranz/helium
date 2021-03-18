const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("helium", "src/main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.install();

    //    const serve = b.step("serve", "start a x server");
    //    serve.makeFn = xeph;

    const run_xeph = addCustom(b, Xephyr{});

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
    run_step.dependOn(&run_xeph.step);
}

const Xephyr = struct {
    step: std.build.Step = undefined,
    pub fn make(step: *std.build.Step) anyerror!void {
        std.debug.print("Test step!\n", .{});
        //const self = @fieldParentPtr(Xephyr, "step", step);
    }
};

//fn xeph(Self: *std.build.Step) !void {
//    std.debug.print("Running xeph", .{});
//    const args = &[_][]const u8{"Xephyr -screen 800x600 :1"};
//    const cp = try std.ChildProcess.init(args, std.heap.page_allocator);
//    cp.spawn() catch |err| {
//        @panic("Oh shit\n");
//    };
//}

const Builder = std.build.Builder;
const Step = std.build.Step;

pub fn addCustom(self: *Builder, customStep: anytype) *@TypeOf(customStep) {
    var allocated = self.allocator.create(@TypeOf(customStep)) catch unreachable;
    allocated.* = customStep;
    allocated.*.step = Step.init(Step.Id.Log, @typeName(@TypeOf(customStep)), self.allocator, @TypeOf(customStep).make);
    return allocated;
}
