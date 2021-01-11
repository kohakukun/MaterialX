// jsUtil
addWrapper(function(Module, api) {
    api.getVersionString = Module.getVersionString;
    api.createValidName = function(str, char) {
        return Module.createValidName(str, char.charCodeAt(0));
    };

    api.makeVersionString = Module.makeVersionString;
    api.isValidName = Module.isValidName;
    api.incrementName = Module.incrementName;

    api.getVersionIntegers = function() {
        var vec = Module.getVersionIntegers();
        return vecToArray(vec);
    };
    api.splitString = function(str, spl) {
        var vecStr = Module.splitString(str, spl);
        var size = vecStr.size();
        var result = [];
        for (var i = 0; i < size; i++) {
            result.push(vecStr.get(i));
        }
        return result;
    };

    api.replaceSubstrings = Module.replaceSubstrings;
    api.prettyPrint = Module.prettyPrint;
});