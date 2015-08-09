$(function() {
	$(".button").hover(
		function () {
			$(this).animate({backgroundColor: "#ECEC89", color: "#333333"});
		},
		function () {
			$(this).animate({backgroundColor: "#F49D57", color: "#282828"});
		}).mousedown(function() {
			$(this).animate({backgroundColor: "#1F1F1F", color: "#909293"});
		}).mouseup(function() {
			$(this).animate({backgroundColor: "#ECEC89", color: "#333333"});
		});
});