/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoa_compat.h
 * @author rdb
 * @date 2023-08-28
 */

#ifndef COCOA_COMPAT_H
#define COCOA_COMPAT_H

#import <Cocoa/Cocoa.h>

// Allow building with older SDKs.
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 110000
typedef NS_ENUM(NSInteger, NSWindowToolbarStyle) {
  NSWindowToolbarStyleAutomatic,
  NSWindowToolbarStyleExpanded,
  NSWindowToolbarStylePreference,
  NSWindowToolbarStyleUnified,
  NSWindowToolbarStyleUnifiedCompact
} API_AVAILABLE(macos(11.0));

API_AVAILABLE(macos(11.0)) API_UNAVAILABLE(ios)
@interface NSTrackingSeparatorToolbarItem : NSToolbarItem
+ (instancetype)trackingSeparatorToolbarItemWithIdentifier:(NSString *)identifier splitView:(NSSplitView *)splitView dividerIndex:(NSInteger)dividerIndex API_UNAVAILABLE(ios);
@property (strong) NSSplitView *splitView API_UNAVAILABLE(ios);
@property NSInteger dividerIndex API_UNAVAILABLE(ios);
@end

#endif  // __MAC_OS_X_VERSION_MAX_ALLOWED

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 101400
@protocol NSViewToolTipOwner <NSObject>
- (NSString *)view:(NSView *)view stringForToolTip:(NSToolTipTag)tag point:(NSPoint)point userData:(nullable void *)data;
@end

#endif  // __MAC_OS_X_VERSION_MAX_ALLOWED

#endif  // COCOA_COMPAT_H
