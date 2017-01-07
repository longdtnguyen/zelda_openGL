
--Define all the material HERE
nontranparent = gr.material({0.99,0.99,0.99},{0.99,0.99,0.99},99,1.0,0)
sail = gr.material({0.5,0.5, 0.5},{0.99,0.99,0.99},99, 0.81,1)	-- material not really matter now, second last is alpha
water = gr.material({0.1,0.2,0.7},{0.99,0.99,0.99},99, 1,3)
white = gr.material({200.0,200.0,200.0},{0.99,0.99,0.99},99,1,2)
pad_col = gr.material({1.0,1.0,1.0},{0.99,0.99,0.99},99,0,2)



----- MA ROOT
rootNode = gr.node('root')



-------------------MA SUN
sun = gr.mesh('sphere','sun')
rootNode:add_child(sun)

sun:set_material(white)
sun:scale(1.5,1.5,1.5)

sun:translate(15.0,8.0,-15.0)

---------------------MA OCEAN
ocean = gr.mesh('water','water')
rootNode:add_child(ocean)

ocean:set_material(water)
ocean:scale(500,1,500)


---------------------MA BOAT
pad = gr.mesh('pad','pad')
rootNode:add_child(pad)
pad:set_material(pad_col)
pad:scale(0.5,0.5,0.5)
-- boat lower body
boatbody = gr.mesh('boatbody', 'boatbody')
pad:add_child(boatbody)

boatbody:set_material(nontranparent)




-- boat sail
boatsail = gr.mesh('boatsail', 'boatsail')
boatbody:add_child(boatsail)

boatsail:set_material(sail)
boatsail:scale(1/0.5,1/0.5,1/0.5)
boatsail:scale(0.5,0.5,0.5)



-- boat dragon head
head = gr.mesh('boathead', 'boathead')
boatbody:add_child(head)

head:set_material(nontranparent)
head:scale(1/0.5,1/0.5,1/0.5)
head:scale(0.5,0.5,0.5)

-- dragon eyes
boateyes = gr.mesh('boateye', 'boateye')
head:add_child(boateyes)

boateyes:set_material(nontranparent)
boateyes:scale(1/0.5,1/0.5,1/0.5)
boateyes:scale(0.5,0.5,0.5)





-- --------------------LINK

--link body
linkbody = gr.mesh('linkbody', 'linkbody')
boatbody:add_child(linkbody)

linkbody:set_material(nontranparent)
linkbody:scale(1/0.5,1/0.5,1/0.5)
linkbody:scale(1.2,1.2,1.2)
linkbody:translate(-0.6,-1.0,-1.2)	


-- ------MOUTH
linkmouth = gr.mesh('linkmouth', 'linkmouth')
linkbody:add_child(linkmouth)

linkmouth:set_material(nontranparent)

-------- LE EYES
lefteye =gr.mesh('lefteye', 'lefteye')
linkbody:add_child(lefteye)

lefteye:set_material(nontranparent)
	

righteye =gr.mesh('righteye', 'righteye')
linkbody:add_child(righteye)

righteye:set_material(nontranparent)


-- link sword
swordsheath = gr.mesh('swordsheath', 'swordsheath')
linkbody:add_child(swordsheath)

swordsheath:set_material(nontranparent)





-- Return the root with all of it's children
return rootNode
