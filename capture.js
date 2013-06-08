#! /usr/bin/env phantomjs

var system = require('system');
var page = require('webpage').create();
var demo = 1;

page.open('index.html', function () {

  for (demo = 1; demo <= 5; demo++) {

    var clipRect = page.evaluate(function(d) {
      var el = document.getElementById('demo' + d);
      //var box = { width: el.width(), height: el.height(), top: el.offset().top, left: el.offset().left };
      var clipRect = el.getBoundingClientRect();
      return clipRect;
    }, demo);

    page.clipRect = clipRect;
    page.paperSize = { format: 'A4', orientation: 'landscape' };
    page.render('captured' + demo + '.png');
  }

  phantom.exit();

});
