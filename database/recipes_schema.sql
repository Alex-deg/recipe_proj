PRAGMA foreign_keys = ON;

CREATE TABLE food_groups (
    id    INTEGER PRIMARY KEY,
    name  TEXT    NOT NULL UNIQUE CHECK (length(name) BETWEEN 1 AND 30)
);

CREATE TABLE allergens (
    id    INTEGER PRIMARY KEY,
    name  TEXT    NOT NULL UNIQUE CHECK (length(name) BETWEEN 1 AND 100)
);

CREATE TABLE tags (
    id     INTEGER PRIMARY KEY,
    label  TEXT    NOT NULL UNIQUE CHECK (length(label) BETWEEN 1 AND 16)
);

CREATE TABLE cuisines (
    id    INTEGER PRIMARY KEY,
    name  TEXT    NOT NULL UNIQUE CHECK (length(name) BETWEEN 1 AND 50)
);

CREATE TABLE courses (
    id    INTEGER PRIMARY KEY,
    name  TEXT    NOT NULL UNIQUE CHECK (length(name) BETWEEN 1 AND 50)
);

CREATE TABLE difficulties (
    id          INTEGER PRIMARY KEY,
    name        TEXT    NOT NULL UNIQUE CHECK (length(name) BETWEEN 1 AND 20),
    sort_order  INTEGER NOT NULL UNIQUE
);


CREATE TABLE cooks (
    id          INTEGER PRIMARY KEY,
    first_name  TEXT    NOT NULL CHECK (length(first_name) BETWEEN 1 AND 30),
    last_name   TEXT    NOT NULL CHECK (length(last_name)  BETWEEN 1 AND 50),
    email       TEXT    NOT NULL UNIQUE CHECK (length(email) <= 100 AND email LIKE '%_@_%'),
    joined_on   TEXT    NOT NULL CHECK (joined_on IS date(joined_on))
);

CREATE TABLE ingredients (
    id             INTEGER PRIMARY KEY,
    name           TEXT    NOT NULL UNIQUE CHECK (length(name) BETWEEN 1 AND 30),
    kcal_100g      REAL    NOT NULL CHECK (kcal_100g    BETWEEN 0 AND 999),
    protein_100g   REAL    NOT NULL CHECK (protein_100g BETWEEN 0 AND 100),
    carbs_100g     REAL    NOT NULL CHECK (carbs_100g   BETWEEN 0 AND 100),
    fat_100g       REAL    NOT NULL CHECK (fat_100g     BETWEEN 0 AND 100),
    fibre_100g     REAL    NOT NULL CHECK (fibre_100g   BETWEEN 0 AND 100),
    food_group_id  INTEGER NOT NULL REFERENCES food_groups (id) ON DELETE RESTRICT
);

CREATE TABLE ingredients_allergens (
    ingredient_id  INTEGER NOT NULL REFERENCES ingredients (id) ON DELETE CASCADE,
    allergen_id    INTEGER NOT NULL REFERENCES allergens   (id) ON DELETE RESTRICT,
    PRIMARY KEY (ingredient_id, allergen_id)
) WITHOUT ROWID;

CREATE TABLE recipes (
    id             INTEGER PRIMARY KEY,
    title          TEXT    NOT NULL CHECK (length(title) BETWEEN 1 AND 120),
    cook_id        INTEGER NOT NULL REFERENCES cooks        (id) ON DELETE RESTRICT,
    servings       INTEGER NOT NULL CHECK (servings > 0),
    prep_minutes   INTEGER NOT NULL CHECK (prep_minutes >= 0),
    cook_minutes   INTEGER NOT NULL CHECK (cook_minutes >= 0),
    cuisine_id     INTEGER NOT NULL REFERENCES cuisines     (id) ON DELETE RESTRICT,
    difficulty_id  INTEGER NOT NULL REFERENCES difficulties (id) ON DELETE RESTRICT,
    course_id      INTEGER NOT NULL REFERENCES courses      (id) ON DELETE RESTRICT,
    published_on   TEXT    NOT NULL CHECK (published_on IS date(published_on)),
    UNIQUE (cook_id, title)
);

CREATE TABLE recipe_ingredients (
    recipe_id      INTEGER NOT NULL REFERENCES recipes     (id) ON DELETE CASCADE,
    ingredient_id  INTEGER NOT NULL REFERENCES ingredients (id) ON DELETE RESTRICT,
    quantity_g     REAL    NOT NULL CHECK (quantity_g > 0),
    is_optional    BOOLEAN NOT NULL DEFAULT 0,
    PRIMARY KEY (recipe_id, ingredient_id)
) WITHOUT ROWID;

CREATE TABLE recipe_tags (
    recipe_id  INTEGER NOT NULL REFERENCES recipes (id) ON DELETE CASCADE,
    tag_id     INTEGER NOT NULL REFERENCES tags    (id) ON DELETE RESTRICT,
    PRIMARY KEY (recipe_id, tag_id)
) WITHOUT ROWID;

CREATE TABLE steps (
    id           INTEGER PRIMARY KEY,
    recipe_id    INTEGER NOT NULL REFERENCES recipes (id) ON DELETE CASCADE,
    step_no      INTEGER NOT NULL CHECK (step_no > 0),
    instruction  TEXT    NOT NULL CHECK (length(instruction) > 0),
    minutes      INTEGER NOT NULL DEFAULT 0 CHECK (minutes >= 0),
    UNIQUE (recipe_id, step_no)
);


-- CREATE INDEX idx_ingredients_food_group       ON ingredients (food_group_id);
-- CREATE INDEX idx_ingredients_allergens_all    ON ingredients_allergens (allergen_id);
-- CREATE INDEX idx_recipes_cuisine              ON recipes (cuisine_id);
-- CREATE INDEX idx_recipes_difficulty           ON recipes (difficulty_id);
-- CREATE INDEX idx_recipes_course               ON recipes (course_id);
-- CREATE INDEX idx_recipe_ingredients_ingr      ON recipe_ingredients (ingredient_id);
-- CREATE INDEX idx_recipe_tags_tag              ON recipe_tags (tag_id);
