const std = @import("std");
usingnamespace @import("c.zig");

pub fn handle_event(evtype: u32) void {
    switch (evtype) {
        0                       => std.debug.warn("EVENT_ERROR", .{}),
        XCB_CREATE_NOTIFY       => std.debug.warn("Window created\n", .{}),
        XCB_DESTROY_NOTIFY      => std.debug.warn("DESTROY\n", .{}),
        XCB_BUTTON_PRESS        => std.debug.warn("BUTTON_PRESS\n", .{}),
        XCB_BUTTON_RELEASE      => std.debug.warn("BUTTON_RELEASE\n", .{}),
        XCB_MOTION_NOTIFY       => std.debug.warn("MOTION\n", .{}),
        XCB_ENTER_NOTIFY        => std.debug.warn("ENTER\n", .{}),
        XCB_LEAVE_NOTIFY        => std.debug.warn("LEAVE\n", .{}),
        XCB_CONFIGURE_NOTIFY    => std.debug.warn("CONFIGURE\n", .{}),
        XCB_KEY_PRESS           => std.debug.warn("KEY_PRESS\n", .{}),
        XCB_FOCUS_IN            => std.debug.warn("FOCUS_IN\n", .{}),
        XCB_FOCUS_OUT           => std.debug.warn("FOCUS_OUT\n", .{}),
        XCB_KEYMAP_NOTIFY       => std.debug.warn("KEYMAP\n", .{}),
        XCB_EXPOSE              => std.debug.warn("EXPOSE\n", .{}),
        XCB_GRAPHICS_EXPOSURE   => std.debug.warn("GRAPHICS_EXPOSURE\n", .{}),
        XCB_NO_EXPOSURE         => std.debug.warn("NO_EXPOSURE\n", .{}),
        XCB_VISIBILITY_NOTIFY   => std.debug.warn("VISIBILITY\n", .{}),
        XCB_UNMAP_NOTIFY        => std.debug.warn("UNMAP\n", .{}),
        XCB_MAP_NOTIFY          => std.debug.warn("MAP\n", .{}),
        XCB_MAP_REQUEST         => std.debug.warn("MAP_REQUEST\n", .{}),
        XCB_REPARENT_NOTIFY     => std.debug.warn("REPARENT\n", .{}),
        XCB_CONFIGURE_REQUEST   => std.debug.warn("CONFIGURE_REQUEST\n", .{}),
        XCB_GRAVITY_NOTIFY      => std.debug.warn("GRAVITY\n", .{}),
        XCB_RESIZE_REQUEST      => std.debug.warn("RESIZE_REQUEST\n", .{}),
        XCB_CIRCULATE_NOTIFY    => std.debug.warn("CIRCULATE\n", .{}),
        XCB_PROPERTY_NOTIFY     => std.debug.warn("PROPERTY\n", .{}),
        XCB_SELECTION_CLEAR     => std.debug.warn("SELECTION_CLEAR\n", .{}),
        XCB_SELECTION_REQUEST   => std.debug.warn("SELECTION_REQUEST\n", .{}),
        XCB_SELECTION_NOTIFY    => std.debug.warn("SELECTION\n", .{}),
        XCB_COLORMAP_NOTIFY     => std.debug.warn("COLORMAP\n", .{}),
        XCB_CLIENT_MESSAGE      => std.debug.warn("CLIENT_MESSAG\nE", .{}),
        XCB_MAPPING_NOTIFY      => std.debug.warn("MAPPING\n", .{}),
        // otherwise print the number
        else => std.debug.warn("Event: {}\n", .{evtype}),
    }
}
