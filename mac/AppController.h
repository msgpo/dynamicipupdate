// Copyright (c) 2009 OpenDNS, LLC. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

extern NSString * PREF_ACCOUNT;
extern NSString * PREF_TOKEN;
extern NSString * PREF_HOSTNAME;
extern NSString * PREF_SEND_UPDATES;
extern NSString * PREF_USER_NETWORKS_STATE;

extern NSString * UNS_OK;
extern NSString * UNS_NO_NETWORKS;
extern NSString * UNS_NO_DYNAMIC_IP_NETWORKS;
extern NSString * UNS_NO_NETWORK_SELECTED;

@interface AppController : NSObject {

	// stuff related to login window
	IBOutlet NSWindow *				windowLogin_;

	IBOutlet NSTextField *			editOpenDnsAccount_;
	IBOutlet NSTextField *			editOpenDnsPassword_;
	
	IBOutlet NSProgressIndicator *	progressLogin_;
	IBOutlet NSTextField *			textLoginProgress_;
	IBOutlet NSTextField *			textLoginError_;

	IBOutlet NSButton *				buttonLogin_;

	// stuff related to select network window
	IBOutlet NSWindow *				windowSelectNetwork_;
	IBOutlet NSTableView *			tableNetworksList_;

	// stuff related to status window
	IBOutlet NSWindow *				windowStatus_;

	// menu-related stuff
	IBOutlet NSMenu *				menu_;
	NSStatusItem *					statusItem_;
	NSImage *						menuIcon_;

	// program state
	NSString *						currentIpAddress_;
	BOOL							exitIpChangeThread_;
}

- (IBAction)login:(id)sender;
- (IBAction)quit:(id)sender;
- (IBAction)loginWindowAbout:(id)sender;
- (IBAction)selectNetworkCancel:(id)sender;
- (IBAction)selectNetworkSelect:(id)sender;
- (IBAction)showStatusWindow:(id)sender;

@end
