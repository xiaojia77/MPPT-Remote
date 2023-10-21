//
//  ItemCell.m
//  OTA_Update
//
//  Created by DFung on 2019/8/21.
//  Copyright © 2019 DFung. All rights reserved.
//

#import "JLDeviceCell.h"

@implementation JLDeviceCell

- (void)awakeFromNib {
    [super awakeFromNib];
    // Initialization code
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated {
    [super setSelected:selected animated:animated];
}

- (void)setIsLinked:(BOOL)isLinked {
    if (isLinked) {
        _subImage.image = [UIImage imageNamed:@"selected"];
    } else {
        _subImage.image = [UIImage imageNamed:@"unselected"];
    }
}


@end
