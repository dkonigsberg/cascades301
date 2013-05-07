import bb.cascades 1.0

Container {
    property alias labelText: rowLabel.text
    property alias valueText: rowValue.text
    visible: rowValue.text && rowValue.text.length > 0
    topMargin: 20
    bottomMargin: 20
    horizontalAlignment: HorizontalAlignment.Fill
    layout: StackLayout {
        orientation: LayoutOrientation.LeftToRight
    }
    Label {
        id: rowLabel
        content.flags: TextContentFlag.ActiveTextOff | TextContentFlag.EmoticonsOff
    }
    Label {
        id: rowValue
        multiline: true
        layoutProperties: StackLayoutProperties {
            spaceQuota: 1
        }
        textStyle.textAlign: TextAlign.Right
        content.flags: TextContentFlag.ActiveText | TextContentFlag.EmoticonsOff
    }
}
