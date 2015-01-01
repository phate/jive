{-
 - Copyright 2015 Nico Reißmann <nico.reissmann@gmail.com>
 - See COPYING for terms of redistribution.
 -}

module RegionTreeTEX where

import Data.MultiMap as MM

import Text.ParserCombinators.Parsec

{- ADTs -}

type Id = String

data RegionTree = RegionTree Id [RegionTree]
	deriving Show

{- Parsing -}

parseId :: Parser Id
parseId =
	many1 (choice [alphaNum, char '_'])

parseDepth :: Parser Int
parseDepth = do
	l <- many.char $ '-'
	return (length l)

parseLine :: Parser (Int, Id)
parseLine = do
	spaces
	d <- parseDepth
	spaces
	id <- parseId
	newline
	return (d, id)

parseLines :: Parser [(Int, Id)]
parseLines =
	many1 parseLine

parseString :: String -> Either String [(Int, Id)]
parseString s =
	case parse parseLines "" s of
		Left err	-> Left (show err)
		Right l 	-> return l

{- Construction -}

construct' :: [(Int, Id)] -> MM.MultiMap Int RegionTree
construct' l =
	Prelude.foldr go MM.empty l
	where
		go (d, id) m  = MM.insert d (RegionTree id (MM.lookup (d+1) m)) (MM.delete (d+1) m)

construct :: [(Int, Id)] -> Either String RegionTree
construct [] =
	Left "Not a valid tree."
construct l =
	case and [(MM.numKeys m) == 1, (MM.numValues m) == 1, MM.member m 0] of
		False -> Left "Not a valid tree."
		True -> Right $ head $ MM.lookup 0 m
	where m = construct' l

{- TEX Serialization -}

escapeChar :: Char -> String
escapeChar '_' = "\\_"
escapeChar c = [c]

escapeId :: Id -> Id
escapeId id =
	concatMap escapeChar id

header :: String
header =
	"\\documentclass[landscape]{article}\n\\usepackage{qtree}\n\\begin{document}\n"

footer :: String
footer =
	"\\end{document}\n"

serializeTEX' :: RegionTree -> String
serializeTEX' (RegionTree id children) =
	"[." ++ escapeId id ++ " " ++ concatMap serializeTEX' children ++ " ]"

serializeTEX :: RegionTree -> String
serializeTEX t =
	header ++ "\\Tree " ++ serializeTEX' t ++ "\n" ++ footer

{- main -}

main :: IO ()
main = do
	s <- getContents
	case parseString s of
		Left err 	-> print err
		Right l		-> case construct l of
									Left err 	-> print err
									Right t		-> putStr $ serializeTEX t
