#import "VGQualityCell.h"
#import "VGTheme.h"


@implementation VGQualityCell

- (instancetype)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
        self.contentView.backgroundColor = VGSurface2;
        self.contentView.layer.cornerRadius = 12;
        self.contentView.layer.cornerCurve = kCACornerCurveContinuous;
        self.contentView.layer.borderWidth = 2;

        _resLabel = [UILabel new];
        _resLabel.font = VGRounded(20, UIFontWeightHeavy);
        _resLabel.textColor = VGText;
        _resLabel.adjustsFontSizeToFitWidth = YES;
        _resLabel.minimumScaleFactor = 0.6;

        _badgeLabel = [UILabel new];
        _badgeLabel.font = VGFont(10, UIFontWeightBold);
        _badgeLabel.textColor = VGAccent;

        _sizeLabel = [UILabel new];
        _sizeLabel.font = VGFont(12, UIFontWeightSemibold);
        _sizeLabel.textColor = VGSecondary;
        _sizeLabel.textAlignment = NSTextAlignmentRight;

        _fmtLabel = [UILabel new];
        _fmtLabel.font = VGFont(9, UIFontWeightHeavy);
        _fmtLabel.textColor = VGTertiary;
        _fmtLabel.textAlignment = NSTextAlignmentRight;

        _check = [[UIImageView alloc] initWithImage:[UIImage systemImageNamed:@"checkmark.circle.fill"]];
        _check.tintColor = VGAccent;

        for (UIView *v in @[_resLabel, _badgeLabel, _sizeLabel, _fmtLabel, _check]) {
            v.translatesAutoresizingMaskIntoConstraints = NO;
            [self.contentView addSubview:v];
        }
        UIView *c = self.contentView;
        [NSLayoutConstraint activateConstraints:@[
            // 左侧：分辨率 + badge
            [_resLabel.leadingAnchor constraintEqualToAnchor:c.leadingAnchor constant:12],
            [_resLabel.centerYAnchor constraintEqualToAnchor:c.centerYAnchor constant:-6],
            [_badgeLabel.leadingAnchor constraintEqualToAnchor:_resLabel.leadingAnchor],
            [_badgeLabel.topAnchor constraintEqualToAnchor:_resLabel.bottomAnchor constant:1],
            // 右侧：文件大小 + 格式
            [_sizeLabel.trailingAnchor constraintEqualToAnchor:c.trailingAnchor constant:-12],
            [_sizeLabel.centerYAnchor constraintEqualToAnchor:c.centerYAnchor constant:-6],
            [_fmtLabel.trailingAnchor constraintEqualToAnchor:_sizeLabel.trailingAnchor],
            [_fmtLabel.topAnchor constraintEqualToAnchor:_sizeLabel.bottomAnchor constant:1],
            [_fmtLabel.leadingAnchor constraintGreaterThanOrEqualToAnchor:_resLabel.trailingAnchor constant:8],
            // 选中标记
            [_check.topAnchor constraintEqualToAnchor:c.topAnchor constant:6],
            [_check.trailingAnchor constraintEqualToAnchor:c.trailingAnchor constant:-6],
            [_check.widthAnchor constraintEqualToConstant:16],
            [_check.heightAnchor constraintEqualToConstant:16],
        ]];
    }
    return self;
}

- (void)configure:(VGOption *)o selected:(BOOL)selected {
    NSArray *parts = [o.res componentsSeparatedByString:@" "];
    self.resLabel.text = parts.firstObject;
    NSString *badge = parts.count > 1 ? [[parts subarrayWithRange:NSMakeRange(1, parts.count - 1)] componentsJoinedByString:@" "].uppercaseString : nil;
    if (o.audio) badge = @"SOUND ONLY";
    else if ([parts.firstObject isEqualToString:@"4K"] || [parts.firstObject isEqualToString:@"2K"]) badge = badge ?: @"ULTRA HD";
    self.badgeLabel.text = badge;
    self.sizeLabel.text = o.sizeText.length ? o.sizeText : @"Size unknown";
    self.fmtLabel.text = o.audio ? o.format : (o.convert ? @"MP4 · CONVERTS" : (o.photos ? [o.format stringByAppendingString:@" · PHOTOS"] : [o.format stringByAppendingString:@" · FILES ONLY"]));
    self.check.hidden = !selected;
    self.contentView.layer.borderColor = selected ? VGOn.CGColor : UIColor.clearColor.CGColor;
    self.contentView.backgroundColor = selected ? VGSelectedBg : VGSurface2;
}

@end

