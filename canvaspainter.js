//Alex Deuling 94357349
//Jared Napoli	31666553
//Noel Biscocho 10062289
//Anthony Serrano 53934109

var canvas;
var ctx;
var w;
var h;
var cw;

function paint_color(y, x, color, stroke_color)
{
	ctx.fillStyle = color;
	ctx.strokeStyle = stroke_color;
	ctx.fillRect(x*cw, y*cw, cw, cw);
	ctx.strokeRect(x*cw, y*cw, cw, cw);
}
function change_height(newH)
{
	h = newH;
    ctx.canvas.height = h;
}
function change_width(newW)
{
	w = newW;
    ctx.canvas.width = w;
}
function random_color()
{
	var red = Math.floor((Math.random() * 100) + 100);
	var green = Math.floor((Math.random() * 100) + 100);
	var blue = Math.floor((Math.random() * 100) + 100);

	return "#"+red.toString(16)+green.toString(16)+blue.toString(16);
}
function clear_screen(color)
{
	ctx.fillStyle = color;
	ctx.strokeStyle = color;
	ctx.fillRect(0, 0, w, h);
	ctx.strokeRect(0, 0, w, h);
}
