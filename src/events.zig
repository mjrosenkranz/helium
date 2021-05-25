const std = @import("std");
const client = @import("client.zig");
const wm = @import("wm.zig");
usingnamespace @import("c.zig");

        // TODO: add to event queue
pub fn handle_event(event: *xcb_generic_event_t) void {
    switch (event.*.response_type & ~@as(u32, 0x80)) {
        0                       => std.debug.warn("EVENT_ERROR", .{}),
        XCB_CREATE_NOTIFY       => handle_create_notify(event),
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
        XCB_MAP_NOTIFY          => std.debug.warn("MAP NOTIFY\n", .{}),
        XCB_MAP_REQUEST         => std.debug.warn("MAP_REQUEST\n", .{}),
        XCB_REPARENT_NOTIFY     => std.debug.warn("REPARENT\n", .{}),
        XCB_CONFIGURE_REQUEST   => handle_configure_request(event),
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
        else => std.debug.warn("Event: {}\n", .{event.*.response_type & ~@as(u32, 0x80)}),
    }
}

fn handle_create_notify(event: *xcb_generic_event_t) void {
    var ev = @ptrCast(*xcb_create_notify_event_t, event).*;
    var geom = client.get_geom(&ev.window);
    std.debug.warn("Create notify handled\n", .{});
}


fn handle_configure_request(event: *xcb_generic_event_t) void {
    var ev = @ptrCast(*xcb_configure_request_event_t, event).*;
    std.debug.warn("configure request handled\n", .{});
    std.debug.warn("x {} y {} w {} h {}\n", .{
        ev.x,
        ev.y,
        ev.width,
        ev.height,
    });

    // sever that shit
    const values = [_]u32{
        ev.x,
        ev.y,
        ev.width,
        ev.height,
        ev.sibling,
        ev.stack_mode,
    };
    xcb_configure_window(wm.conn, ev.window, ev.mask, values);
    xcb_flush(wm.conn);
}
