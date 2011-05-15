import Qt 4.7

TextEdit {
    signal messageSubmitted(string text)

    function submitMessage() {
        if (text.length == 0)
            return

        messageSubmitted(text)
        text = ""
    }

    Keys.priority: Keys.BeforeItem
    Keys.onEnterPressed: {
        if (event.modifiers & (Qt.ShiftModifier | Qt.ControlModifier))
            event.accepted = false
        else
            submitMessage()
    }
    Keys.onReturnPressed: {
        if (event.modifiers & (Qt.ShiftModifier | Qt.ControlModifier))
            event.accepted = false
        else
            submitMessage()
    }
}
