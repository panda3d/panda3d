/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file macStatsGraphViewController.mm
 * @author rdb
 * @date 2023-08-28
 */

#include "macStatsGraphViewController.h"
#include "macStatsGraph.h"
#include "macStatsMonitor.h"
#include "cocoa_compat.h"

@implementation MacStatsGraphViewController

- (id)initWithGraph:(MacStatsGraph *)graph {
  if (self = [super init]) {
    _graph = graph;
  }

  return self;
}

- (void)windowWillClose:(NSNotification *)notification {
  MacStatsGraph *graph = _graph;
  if (graph != nullptr) {
    MacStatsMonitor *monitor = graph->get_monitor();
    if (monitor != nullptr) {
      _graph = nullptr;
      monitor->remove_graph(graph);
    }
  }
}

- (void)loadView {
  NSView *graph_view = [[MacStatsGraphView alloc] initWithGraph:_graph];
  NSView *background;
  if (@available(macOS 10.14, *)) {
    NSVisualEffectView *effect_view = [[NSVisualEffectView alloc] init];
    effect_view.material = (NSVisualEffectMaterial)18;//NSVisualEffectMaterialContentBackground;
    background = effect_view;
  } else {
    background = [[NSView alloc] init];
    background.wantsLayer = YES;
    background.layer.backgroundColor = [NSColor controlBackgroundColor].CGColor;
  }
  [background addSubview:graph_view];
  self.view = background;

  [graph_view.widthAnchor constraintEqualToAnchor:background.widthAnchor].active = YES;
  [graph_view.heightAnchor constraintEqualToAnchor:background.heightAnchor].active = YES;
  [graph_view release];
  [background release];
}

- (MacStatsGraphView *)graphView {
  return (MacStatsGraphView *)self.view.subviews[0];
}

- (void)handleSplitViewResize:(NSNotification *)notification {
  NSSplitView *split_view = (NSSplitView *)notification.object;
  NSWindow *window = split_view.window;
  NSToolbar *toolbar = window.toolbar;
  if ([split_view isSubviewCollapsed:split_view.arrangedSubviews[0]]) {
    if (toolbar.items[0].itemIdentifier != NSToolbarToggleSidebarItemIdentifier) {
      [toolbar insertItemWithItemIdentifier:NSToolbarToggleSidebarItemIdentifier atIndex:0];
    }
  } else {
    if (toolbar.items[0].itemIdentifier == NSToolbarToggleSidebarItemIdentifier) {
      [toolbar removeItemAtIndex:0];
    }
  }
}

- (BOOL)backToolbarItemVisible {
  NSToolbar *toolbar = self.view.window.toolbar;
  if ([toolbar.items[0].itemIdentifier isEqual:@"back"] ||
      [toolbar.items[1].itemIdentifier isEqual:@"back"]) {
    return YES;
  } else {
    return NO;
  }
}

- (void)setBackToolbarItemVisible:(BOOL)show {
  NSToolbar *toolbar = self.view.window.toolbar;
  if ([toolbar.items[1].itemIdentifier isEqual:@"back"]) {
    if (!show) {
      [toolbar removeItemAtIndex:1];
    }
  }
  else if ([toolbar.items[0].itemIdentifier isEqual:@"back"]) {
    if (!show) {
      [toolbar removeItemAtIndex:0];
    }
  }
  else if (show) {
    // Insert it after the sidebar toggle, if we have one.
    int index = ([toolbar.items[0].itemIdentifier isEqual:NSToolbarToggleSidebarItemIdentifier]);
    [toolbar insertItemWithItemIdentifier:@"back" atIndex:index];
  }
}

- (NSArray *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar {
  return @[];
}

- (NSArray *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar {
  return @[];
}

- (NSToolbarItem *) toolbar:(NSToolbar *)toolbar
      itemForItemIdentifier:(NSString *)ident
  willBeInsertedIntoToolbar:(BOOL)flag {

  if (@available(macOS 11.0, *)) {
    if ([ident isEqual:@"sep"]) {
      return [NSTrackingSeparatorToolbarItem trackingSeparatorToolbarItemWithIdentifier:@"sep" splitView:_graph->get_split_view() dividerIndex:0];
    }

    if ([ident isEqual:@"back"]) {
      NSToolbarItem *item = [[[NSToolbarItem alloc] initWithItemIdentifier:ident] autorelease];
      item.label = @"Back";
      item.image = [NSImage imageWithSystemSymbolName:@"chevron.left" accessibilityDescription:@""];
      item.target = self;
      item.action = @selector(handleBack:);
      [item setNavigational:YES];
      [item setBordered:YES];
      return item;
    }
  }

  return nil;
}

- (void)handleSetAsFocus:(NSMenuItem *)item {
  _graph->on_click_label(item.tag);
}

- (void)handleChangeColor:(NSMenuItem *)item {
  _graph->get_monitor()->choose_collector_color(item.tag);
}

- (void)handleResetColor:(NSMenuItem *)item {
  _graph->get_monitor()->reset_collector_color(item.tag);
}

- (void)handleBack:(id)sender {
  _graph->handle_back();
}

@end

@implementation MacStatsScrollableGraphViewController

- (void)loadView {
  NSView *graph_view = [[MacStatsGraphView alloc] initWithGraph:_graph];
  NSScrollView *scroll = [[NSScrollView alloc] init];
  scroll.hasHorizontalScroller = NO;
  scroll.hasVerticalScroller = YES;
  scroll.horizontalScrollElasticity = NSScrollElasticityNone;
  scroll.usesPredominantAxisScrolling = NO;
  scroll.drawsBackground = YES;
  scroll.scrollerStyle = NSScrollerStyleOverlay;
  scroll.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
  //scroll.translatesAutoresizingMaskIntoConstraints = NO;
  scroll.automaticallyAdjustsContentInsets = YES;
  scroll.documentView = graph_view;
  self.view = scroll;

  [graph_view.widthAnchor constraintEqualToAnchor:scroll.widthAnchor].active = YES;

  if (@available(macOS 11.0, *)) {
    [graph_view.heightAnchor constraintGreaterThanOrEqualToAnchor:((NSLayoutGuide *)[scroll safeAreaLayoutGuide]).heightAnchor].active = YES;
  } else {
    [graph_view.heightAnchor constraintGreaterThanOrEqualToAnchor:scroll.heightAnchor].active = YES;
  }

  NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
  [center addObserver:self
           selector:@selector(handleScroll:)
               name:NSScrollViewDidLiveScrollNotification
             object:scroll];
  [scroll release];
  [graph_view release];
}

- (MacStatsGraphView *)graphView {
  return (MacStatsGraphView *)((NSScrollView *)self.view).documentView;
}

- (NSClipView *)clipView {
  return ((NSScrollView *)self.view).contentView;
}

- (void)viewDidLayout {
  if (_graph != nullptr) {
    _graph->handle_scroll();
  }
}

- (void)handleScroll:(NSNotification *)notification {
  if (_graph != nullptr) {
    _graph->handle_scroll();
  }
}

- (void)handleSideScroll:(NSNotification *)notification {
  // Graph view is flipped, side bar isn't, so we need to convert coordinates
  NSScrollView *side_sv = ((NSScrollView *)notification.object);
  NSScrollView *graph_sv = (NSScrollView *)self.view;
  NSPoint point;
  point.x = 0;
  point.y = self.graphView.frame.size.height - (side_sv.documentVisibleRect.size.height + side_sv.documentVisibleRect.origin.y) - graph_sv.contentInsets.top;
  [graph_sv.contentView scrollToPoint:point];
  [graph_sv reflectScrolledClipView:graph_sv.contentView];
}

@end
