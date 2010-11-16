#include "matwm.h"

#define iskey(k) ((ev.xkey.state == k.mask || ev.xkey.state == (k.mask | numlockmask) || ev.xkey.state == (k.mask | LockMask) || ev.xkey.state == (k.mask | LockMask | numlockmask)) && ev.xkey.keycode == k.code)

void handle_event(XEvent ev) {
  int c, i;
  for(c = 0; c < cn; c++)
    if(clients[c].parent == ev.xany.window)
      break;
  switch(ev.type) {
    case MapRequest:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xmaprequest.window)
          break;
      if(i < cn) {
        if(clients[c].iconic)
          restore(c);
      } else add_client(ev.xmaprequest.window);
      break;
    case DestroyNotify:
#ifdef DEBUG
      if(c < cn && clients[c].window != ev.xdestroywindow.window)
        printf("destroynotify (%s) && xany.window == parent - but window != xdestroywindow.window, Why is this? has x11 gone insane?\n", clients[c].name);
#endif
      if(c < cn && clients[c].window == ev.xdestroywindow.window)
        remove_client(c, 2);
      break;
    case UnmapNotify:
#ifdef DEBUG
      if(c < cn && clients[c].window != ev.xunmap.window)
        printf("unmapnotify (%s) && xany.window == parent - but window != xunmap.window, Why is this? has x11 gone insane?\n", clients[c].name);
#endif
      if(c < cn && clients[c].window == ev.xunmap.window)
        remove_client(c, 1);
      break;
    case ConfigureRequest:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xconfigurerequest.window)
          break;
      if(i < cn) {
        resize(i, (ev.xconfigurerequest.value_mask & CWWidth) ? ev.xconfigurerequest.width : clients[i].width, (ev.xconfigurerequest.value_mask & CWHeight) ? ev.xconfigurerequest.height : clients[i].height);
        move(i, (ev.xconfigurerequest.value_mask & CWX) ? ev.xconfigurerequest.x - gxo(i, 0) : clients[i].x, (ev.xconfigurerequest.value_mask & CWY) ? ev.xconfigurerequest.y - gyo(i, 0) : clients[i].y);
      } else {
        XWindowChanges wc;
        wc.sibling = ev.xconfigurerequest.above;
        wc.stack_mode = ev.xconfigurerequest.detail;
        wc.x = ev.xconfigurerequest.x;
        wc.y = ev.xconfigurerequest.y;
        wc.width = ev.xconfigurerequest.width;
        wc.height = ev.xconfigurerequest.height;
        XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &wc);
      }
      break;
    case PropertyNotify:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xproperty.window)
          break;
     if(i < cn && ev.xproperty.atom == XA_WM_NAME) {
        if(clients[i].name)
          XFree(clients[i].name);
        XFetchName(dpy, clients[i].window, &clients[i].name);
        XClearWindow(dpy, clients[i].parent);
        draw_client(i);
      }
      if(ev.xproperty.atom == XA_WM_NORMAL_HINTS && i < cn)
        getnormalhints(i);
      if(ev.xproperty.atom == xa_motif_wm_hints && i < cn) {
        get_mwm_hints(i);
        XMoveWindow(dpy, clients[i].window, border(i), border(i) + title(i));
        XResizeWindow(dpy, clients[i].parent, (clients[i].iconic && !clients[i].transient) ? icon_width : (clients[i].width + (border(i) * 2)), (clients[i].iconic && !clients[i].transient) ? title_height + 4 : (clients[i].height + (border(i) * 2) + title(i)));
      }
      break;
    case ClientMessage:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xclient.window)
          break;
      if(i < cn && ev.xclient.message_type == xa_wm_change_state && ev.xclient.data.l[0] == IconicState && !clients[i].transient)
        iconify(i);
      break;
    case EnterNotify:
      if(c == cn)
        for(c = 0; c < cn; c++)
          if(clients[c].window == ev.xcrossing.window)
            break;
      if(c < cn)
        focus(c);
      break;
    case Expose:
      if(c < cn && ev.xexpose.count == 0)
        draw_client(c);
      break;
    case ButtonPress:
      if(c < cn && !clients[c].iconic) {
        if(strcmp(buttonaction(c, ev.xbutton.button), "move") == 0)
          drag(&ev.xbutton, 0);
        if(strcmp(buttonaction(c, ev.xbutton.button), "resize") == 0 && clients[c].resize)
          drag(&ev.xbutton, 1);
      }
      break;
    case ButtonRelease:
      if(c < cn)
        if(strcmp(buttonaction(c, ev.xbutton.button), "raise") == 0) {
          clients[c].iconic ? restack_icons(1) : restack_client(c, 1);
        } else if(strcmp(buttonaction(c, ev.xbutton.button), "lower") == 0) {
          clients[c].iconic ? restack_icons(0) : restack_client(c, 0);
        } else if(clients[c].iconic && strcmp(buttonaction(c, ev.xbutton.button), "restore") == 0)
          restore(c);
      break;
    case MappingNotify:
      if(ev.xmapping.request != MappingPointer) {
        XUngrabKeyboard(dpy, CurrentTime);
        XRefreshKeyboardMapping(&ev.xmapping);
        mapkeys();
      }
      break;
    case KeyPress:
      if(current < cn && iskey(key_close))
        delete_window(current);
      if(iskey(key_next))
        next(0);
      if(iskey(key_prev))
        prev(0);
      if(iskey(key_next_icon)) {
        next(1);
        restack_icons(1);
      }
      if(iskey(key_prev_icon)) {
        prev(1);
        restack_icons(1);
      }
      if(current < cn && iskey(key_iconify) && !clients[current].transient) {
        icons_ontop = 0;
        clients[current].iconic ? restore(current) : iconify(current);
        if(!clients[current].iconic)
          warp();
      }
      if(current < cn && iskey(key_maximise)) {
        if(clients[current].iconic)
          restore(current);
        maximise(current);
      }
      if(current < cn && iskey(key_bottomleft))
        move(current, 0, display_height - (clients[current].height + (border(current) * 2) + title(current)));
      if(current < cn && iskey(key_bottomright))
        move(current, display_width - (clients[current].width + (border(current) * 2)), display_height - (clients[current].height + (border(current) * 2) + title(current)));
      if(current < cn && iskey(key_topright))
        move(current, display_width - (clients[current].width + (border(current) * 2)), 0);
      if(current < cn && iskey(key_topleft))
        move(current, 0, 0);
      break;
    default:
      for(i = 0; i < cn; i++)
        if(clients[i].window == ev.xany.window)
          break;
      if(i < cn && ev.type == shape_event)
        set_shape(i);
      break;
  }
}

