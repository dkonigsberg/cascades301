import bb.cascades 1.0

Page {
    id: page
    property bool hasData: false
    property bool loadingData: false
    signal loadData()
    signal showDetails(variant dataItem)
    Container {
        layout: DockLayout {
        }
        ImageView {
            visible: !hasData
            imageSource: "asset:///images/house-logo.png"
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
        }
        ActivityIndicator {
            id: activityIndicator
            running: page.loadingData
            preferredWidth: 400
            preferredHeight: 400
            horizontalAlignment: HorizontalAlignment.Center
            verticalAlignment: VerticalAlignment.Center
        }
        ListView {
            id: listView
            objectName: "listView"
            visible: !activityIndicator.running && hasData
            listItemComponents: [
                ListItemComponent {
                    type: "item"
                    StandardListItem {
                        title: ListItemData.displayName
                        description: ListItemData.role
                        status: ListItemData.party
                        image: ListItemData.photo
                    }
                }
            ]
            onTriggered: {
                var chosenItem = dataModel.data(indexPath)
                if(!chosenItem.loading) {
                    page.showDetails(chosenItem)
                }
            }
        }
    }
    actions: [
        ActionItem {
            title: qsTr("Load Data")
            enabled: !page.loadingData
            ActionBar.placement: ActionBarPlacement.OnBar
            onTriggered: {
                page.loadData()
            }
        }
    ]
}
