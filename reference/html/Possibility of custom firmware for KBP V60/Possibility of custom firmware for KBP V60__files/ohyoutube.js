/*
 Copyright (c) 2015 Jessica GonzÃ¡lez
 @license http://www.mozilla.org/MPL/MPL-1.1.html
*/

$( function() {
    function b() {
        $( ".youtube" ).each(function() {
            $( this ).css("background-image", "url(https://i.ytimg.com/vi/" + this.id + "/sddefault.jpg)");
	})
        $( ".youtube_play" ).each(function() {
            $( document ).on("click", "#" + this.id, function(a) {
                a = "https://www.youtube.com/embed/" + this.id + "?autoplay=1&autohide=1";
                $( this ).data("params") && (a += "&" + $( this ).data("params"));
                a = $("<iframe/>", {
                    frameborder: "0",
                    src: a,
                    width: $( this ).width(),
                    height: $( this ).height()
                });
                $( this ).replaceWith(a)
            })
        })
    }
    b();
    var c = function(a) {
        for (var b =
        window.location.search.substring(1).split(";"), d = 0; d < b.length; d++) {
            var c = b[d].split("=");
            if (c[0] == a)
                return c[1]
        }
    }("action");
    if (c && "post" == c)
        $( "input[name=preview]" ).on("click", function() {
            setInterval(function() {
                b()
            }, 3E3)
        })
} );



