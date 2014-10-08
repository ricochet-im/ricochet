.pragma library

var windows = { }
var createWindow = function() { console.log("BUG!") }

function getWindow(user) {
    var id = user.uniqueID
    var window = windows[user.uniqueID]

    if (window === undefined || window === null) {
        window = createWindow(user)
        window.closed.connect(function() { windows[id] = undefined })
        windows[id] = window
    }
    return window
}

function windowExists(user) {
    return windows[user.uniqueID] !== undefined && windows[user.uniqueID] !== null
}
