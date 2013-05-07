import bb.cascades 1.0

RepListItemBase {
    id: item
    property string party
    property alias image: imageView.image
    property alias imageSource: imageView.imageSource
    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property alias status: statusLabel.text
    property bool pressed: false
    signal triggered()
    leftPadding: edgePadding
    rightPadding: edgePadding
    topMargin: edgePadding
    bottomMargin: edgePadding
    layout: DockLayout { }
    Container {
        topPadding: edgePadding
        bottomPadding: edgePadding
        leftPadding: edgePadding
        rightPadding: edgePadding
        background: {
            if(party == "R") {
                repBackgroundColor
            } else if(party == "D") {
                demBackgroundColor
            } else {
                indBackgroundColor
            }
        }
        horizontalAlignment: HorizontalAlignment.Fill
        verticalAlignment: VerticalAlignment.Center
        layout: StackLayout {
            orientation: LayoutOrientation.LeftToRight
        }
        ImageView {
            id: imageView
            scalingMethod: ScalingMethod.AspectFill
            preferredHeight: photoSize
            preferredWidth: photoSize
            rightMargin: 20
            verticalAlignment: VerticalAlignment.Center
        }
        Container {
            leftMargin: 20
            preferredHeight: photoSize
            verticalAlignment: VerticalAlignment.Center
            layoutProperties: StackLayoutProperties {
                spaceQuota: 1
            }
            Label {
                id: titleLabel
                topMargin: 0
                bottomMargin: 0
                textStyle {
                    base: SystemDefaults.TextStyles.TitleText
                    color: primaryTextColor
                }
            }
            Label {
                id: descriptionLabel
                topMargin: 0
                bottomMargin: 0
                textStyle {
                    base: SystemDefaults.TextStyles.SubtitleText
                    color: secondaryTextColor
                }
            }
            Container {
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 1
                }
            }
            Label {
                id: statusLabel
                topMargin: 0
                bottomMargin: 0
                horizontalAlignment: HorizontalAlignment.Right
                textStyle {
                    base: SystemDefaults.TextStyles.SubtitleText
                    color: secondaryTextColor
                }
            }
        }
    }
    ImageView {
        visible: pressed
        imageSource: "asset:///images/border-listitem.amd"
        verticalAlignment: VerticalAlignment.Fill
        horizontalAlignment: HorizontalAlignment.Fill
    }
    onTouch: {
        if (event.isDown() || event.isMove()) {
            pressed = true;
        } else if (event.isUp() && pressed) {
            pressed = false;
            item.triggered()
        } else {
            pressed = false;
        }
    }
}
