import bb.cascades 1.0

Page {
    id: page
    property alias photo: photoView.image
    property alias displayName: nameLabel.text
    property alias role: roleLabel.text
    property alias party: partyRow.valueText
    property alias notes: notesRow.valueText
    property alias website: websiteRow.valueText
    property alias phoneNumber: phoneRow.valueText
    property alias address: addressRow.valueText
    property alias committees: committeesRow.valueText
    signal showFullPhoto()
    Container {
        topPadding: 10
        leftPadding: 10
        rightPadding: 10
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            ImageView {
                id: photoView
                preferredWidth: 200
                preferredHeight: 200
                scalingMethod: ScalingMethod.AspectFit
                gestureHandlers: [
                    TapHandler {
                        onTapped: {
                            page.showFullPhoto()
                        }
                    }
                ]
            }
            Container {
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 1
                }
                verticalAlignment: VerticalAlignment.Center
                leftMargin: 20
                Label {
                    id: nameLabel
                    textStyle.base: SystemDefaults.TextStyles.TitleText
                }
                Label {
                    id: roleLabel
                    textStyle.base: SystemDefaults.TextStyles.BodyText
                }
            }
        }
        Divider { }
        DetailRow {
            id: partyRow
            labelText: qsTr("Party:")
        }
        DetailRow {
            id: committeesRow
            labelText: qsTr("Committees:")
        }
        DetailRow {
            id: websiteRow
            labelText: qsTr("Website:")
        }
        DetailRow {
            id: phoneRow
            labelText: qsTr("Phone number:")
        }
        DetailRow {
            id: addressRow
            labelText: qsTr("Address:")
        }
        DetailRow {
            id: notesRow
            labelText: qsTr("Notes:")
        }
    }
}
