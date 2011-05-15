import org.torsionim.torsion 1.0
import Qt 4.7
import "switcher.js" as Switcher

PageSwitcherBase {
    id: switcher

    property variant currentPage
    property Item currentItem
    property Component delegate

    function setCurrentPage(key) {
        var item = Switcher.pages[key]
        if (item === undefined) {
            item = delegate.createObject(parent)
            Switcher.pages[key] = item
        }

        if (item == currentItem)
            return

        if (currentItem) {
            setAttachedProperty(currentItem, "isCurrentItem", false)
        }

        currentPage = key
        currentItem = item
        setAttachedProperty(currentItem, "isCurrentItem", true)
    }

    function deletePage(key) {
        var item = Switcher.pages[key]
        if (item === undefined)
            return

        if (item == currentItem) {
            currentItem = undefined
            currentPage = undefined
        }

        item.destroy()
        Switcher.pages[key] = undefined
    }
}
